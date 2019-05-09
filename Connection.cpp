//
// Created by skyitachi on 2019-04-23.
//

#include "Connection.h"

namespace socks {

static char fixedHeader[8] = {0x00, 0x5a, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47};

static void on_proxy_uv_alloc(uv_handle_t* handle, size_t suggest_size, uv_buf_t* buf) {
  Connection* conn = (Connection* )handle->data;
  assert(conn);
  buf->base = conn->upstreamBuf;
  buf->len = sizeof(conn->upstreamBuf);
}

static void on_connection_write_end(uv_write_t* req, int status) {
  Connection* conn = (Connection *)req->data;
  assert(conn);
  free(req);
  if (status < 0) {
    printf("on_connection_write_end error %s\n", uv_strerror(status));
    return;
  }
}

static void on_write_to_upstream_done(uv_write_t* req, int status) {
  Connection* conn = (Connection *)req->data;
  assert(conn);
  free(req);
  if (status < 0) {
    if (conn->status == Connection::SERVER_CLOSE) {
      printf("on_write_to_upstream_done freeRemoteTcp\n");
      conn->freeRemoteTcp();
    }
    return;
  }
  conn->status = Connection::CONNECTED;
  conn->clientOffset = 0;
  conn->pendingLen = 0;
}

// proxy read upstream
static void on_proxy_uv_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* uvBuf) {
  Connection* conn = (Connection*) stream->data;
  assert(conn);
  printf("in the on_proxy_uv_read read %zu\n", nread);
  if (nread < 0) {
    printf("on_proxy_uv_read error %s\n", uv_strerror(nread));
    if (conn->status == Connection::CONNECTED) {
      conn->status = Connection::SERVER_CLOSE;
      // 如果是http的话, 这里必须清理
      printf("freeRemoteTcp on_proxy_uv_read\n");
      conn->freeRemoteTcp();
    }
    return;
  }
  if (nread > 0) {
    conn->writeToClient(uvBuf->base, nread);
  }
}

static void on_connect_to_upstream(uv_connect_t* req, int status) {
  Connection* conn = (Connection *)req->data;
  assert(conn);
  if (status < 0) {
    conn->status = Connection::SERVER_CONNECT_ERROR;
    return;
  }
  // start write handshake data, connected then reply header
  // https://www.openssh.com/txt/socks4.protocol
  conn->sendHeader();

  if (status == Connection::DATA_PENDING) {
    conn->writeToProxy(conn->buf + conn->clientOffset, conn->pendingLen);
  }
  printf("connect to remote successfully\n");
  uv_read_start(conn->upstream(), on_proxy_uv_alloc, on_proxy_uv_read);
  conn->status = Connection::CONNECTED;
}

void Connection::write(char* buf, size_t len, uv_stream_t* stream, uv_write_cb cb) {
  uv_buf_t uvBuf;
  uvBuf = uv_buf_init(buf, len);
  uv_write_t* writeReq = (uv_write_t* )malloc(sizeof(uv_write_t));
  writeReq->data = this;
  uv_write(writeReq, stream, &uvBuf, 1, cb);
}

void Connection::writeToClient(char *buf, size_t len) {
  write(buf, len, stream(), on_connection_write_end);
}

// proxy writes data to upstream
void Connection::writeToProxy(char *buf, size_t len) {
  if (status != SERVER_FREED) {
    // 只要server没被销毁，都要发送
    write(buf, len, upstream(), on_write_to_upstream_done);
  } else {
    printf("server has been freed before write\n");
  }
}

void Connection::sendHeader() {
  writeToClient(fixedHeader, sizeof(fixedHeader));
}

void Connection::connectToRemote(const char *ip, uint16_t port) {
  struct sockaddr_in sockaddrIn;
  uv_ip4_addr(ip, port, &sockaddrIn);
  uv_tcp_connect(connectReq, remoteTcp, (const struct sockaddr*)&sockaddrIn, on_connect_to_upstream);
}

void Connection::onData(char* receiveBuf, size_t len) {
  printf("receive data bytes %zu, conn status %d\n", len, status);
  if (len < SOCKS4A_HEADER_LENGTH && status == INIT) {
    clientOffset += len;
    return;
  }
  // client connect request
  if (receiveBuf[0] == 0x04 && receiveBuf[1] == 0x01 && status == INIT) {
    status = CLIENT_REQUEST;
    remoteAddr = util::parseAddr(receiveBuf, 2);
    printf("remote address is %s:%d\n", remoteAddr.ip, remoteAddr.port);

    connectToRemote(remoteAddr.ip, remoteAddr.port);

    // read username
    size_t userNameLen = strlen(receiveBuf + SOCKS4A_HEADER_LENGTH - 1);
    if (userNameLen > 0) {
      printf("user %s connecting to proxy\n", receiveBuf + SOCKS4A_HEADER_LENGTH - 1);
    }
    if (len > userNameLen + SOCKS4A_HEADER_LENGTH) {
      // extract data buf
      clientOffset = userNameLen  + SOCKS4A_HEADER_LENGTH;
      pendingLen = len - clientOffset;
      status = DATA_PENDING;
    } else {
      clientOffset = 0;
      pendingLen = 0;
    }
  } else if (status == CONNECTED) {
    writeToProxy(receiveBuf, len);
  } else if (status == DATA_PENDING) {
    pendingLen += len;
  } else if (status == CLIENT_REQUEST) {
    // 保留在缓冲区的buffer
    clientOffset = len;
    pendingLen = len;
  } else {
    printf("unexpected connection status %d\n", status);
  }
}

};
