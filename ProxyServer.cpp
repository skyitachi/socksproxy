//
// Created by skyitachi on 2019-04-23.
//

#include "ProxyServer.h"

static char handshake[8] = {0x00, 0x5a, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47};
static uv_buf_t uvBufHandshake = uv_buf_init(handshake, sizeof(handshake));

static int readPort(const char *buf, ssize_t offset) {
  return ((unsigned char)buf[0 + offset] << 8) + (unsigned char)buf[1 + offset];
}

static Addr* parseAddr(const char *buf, ssize_t offset) {
  Addr *addr = (Addr *)malloc(sizeof(Addr));
  addr->port = readPort(buf, offset);
  sprintf(addr->ip, "%d.%d.%d.%d",
          (unsigned char)buf[2+offset],
          (unsigned char)buf[3+offset],
          (unsigned char)buf[4+offset],
          (unsigned char)buf[5+offset]
  );
  return addr;
}

static void on_uv_client_alloc(uv_handle_t *handle, size_t suggest_size, uv_buf_t *buf) {
  Connection* conn = (Connection* )handle->data;
  assert(conn);
  buf->base = conn->buf + conn->clientOffset;
  buf->len = sizeof(conn->buf) - conn->clientOffset;
  if (buf->len < 0) {
     buf->len = 0;
  }
}

static void on_proxy_uv_alloc(uv_handle_t* handle, size_t suggest_size, uv_buf_t* buf) {
  Connection* conn = (Connection* )handle->data;
  assert(conn);
  buf->base = conn->upstreamBuf;
  buf->len = sizeof(conn->upstreamBuf);
}

static void on_proxy_client_write_done(uv_write_t* req, int status) {
  if (status < 0) {
    printf("proxy_write client error: %s\n", uv_strerror(status));
    return;
  }
  Connection *conn = (Connection* ) req->data;
  assert(conn);
  conn->clientOffset = 0;
  printf("proxy_write right\n");

}

static void on_proxy_write_done(uv_write_t* req, int status) {
  if (status < 0) {
    printf("proxy_write error: %s\n", uv_strerror(status));
    return;
  }
  printf("proxy_write right\n");
}

static void on_proxy_uv_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* uvBuf) {
  if (nread > 0) {
    Connection* conn = (Connection*) stream->data;
    assert(conn);
    // NOTE: should set buf length
    uv_buf_t tmp = uv_buf_init(uvBuf->base, nread);
    uv_write(conn->writeReq, conn->stream(), &tmp, 1, on_proxy_write_done);
  }
}

// proxy connect to server
static void on_uv_connect(uv_connect_t* req, int status) {
  if (status < 0) {
    printf("connect error: %s\n", uv_strerror(status));
    return;
  }
  Connection *conn = (Connection* )req->data;
  assert(conn);
  printf("proxy connect to server successfully\n");
  uv_read_start(conn->upstream(), on_proxy_uv_alloc, on_proxy_uv_read);
  // connected
  conn->status = 1;
  if (conn->clientOffset) {
    printf("current buffer is sending out %zd\n", conn->clientOffset);
    uv_buf_t buffered = uv_buf_init(conn->buf, conn->clientOffset);
    uv_write(conn->writeReq, conn->upstream(), &buffered, 1, on_proxy_client_write_done);
  }
}

// proxy read client data
static void on_uv_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* uvBuf) {
  if (nread < 0) {
    // TODO: error happens
    printf("read error: %s\n", uv_strerror(nread));
    return;
  }
  Connection* conn = (Connection *)stream->data;
  assert(conn);
  if (!conn->status && nread < SOCKS4A_HEADER_LENGTH + 1) {
    printf("receive too small data\n");
    conn->clientOffset += nread;
    return;
  }
  // socks4a proxy
  // client connect packet
  if (uvBuf->base[0] == 0x04 && uvBuf->base[1] == 0x01) {
    printf("client request received\n");
    // TODO: parse remote address
    Addr *addr = parseAddr(uvBuf->base, 2);
    printf("remote address is %s:%d\n", addr->ip, addr->port);
    struct sockaddr_in dest;
    uv_ip4_addr(addr->ip, addr->port, &dest);
    uv_tcp_connect(conn->connectReq, conn->remoteTcp, (const struct sockaddr* )&dest, on_uv_connect);

    // start write hanshake data
    uv_write(conn->writeReq, conn->stream(), &uvBufHandshake, 1, on_proxy_write_done);
  } else {
    printf("receive payload: %ld bytes\n", nread);
    // transfer data
    if (conn->status) {
      if (conn->clientOffset > 0) {
        // 有些数据还在 buf里，需要即时发送
        uv_buf_t tmp = uv_buf_init(conn->buf, nread + conn->clientOffset);
        uv_write(conn->writeReq, conn->upstream(), &tmp, 1, on_proxy_client_write_done);
      } else {
        // NOTE: proxy send data to upstream server
        uv_buf_t tmp = uv_buf_init(uvBuf->base, nread);
        uv_write(conn->writeReq, conn->upstream(), &tmp, 1, on_proxy_write_done);
      }
    } else {
      // TODO: 超出 buffer size限制的情况
      conn->clientOffset += nread;
      printf("receive bytes just in buffer\n");
    }
  }
}

void ProxyServer::onConnection_(Connection* tunnel) {
  for(auto listener: listener_) {
    listener(tunnel);
  }
}

void ProxyServer::addConnectionListener(ProxyServer::ConnectionListener listener) {
  listener_.push_back(listener);
}

// static
// libuv callback cannot be class member function
void ProxyServer::on_uv_connection(uv_stream_t* server, int status) {
  if (status < 0) {
    printf("on_new_connection error %s\n", uv_strerror(status));
    return;
  }
  ProxyServer* serverCtx = (ProxyServer* ) server->data;
  assert(serverCtx);
  Connection* conn = new Connection();
  if (!uv_accept(server, conn->stream())) {
    // connection accepted
    uv_read_start(conn->stream(), on_uv_client_alloc, on_uv_read);
  }

}

int ProxyServer::listen(uint16_t port) {
  assert(loop_);
  uv_tcp_init(loop_, &server_);
  const char* host = "0.0.0.0";
  sockaddr_in addr;
  uv_ip4_addr(host, port, &addr);
  uv_tcp_bind(&server_, (const sockaddr *)&addr, 0);
  server_.data = this;
  // 将实例绑定到server_.data里应该也是可以的
  int r = uv_listen((uv_stream_t *)&server_, 1, on_uv_connection);
  if (r < 0) {
    return r;
  }
  printf("listened ok\n");
  // TODO: 不应该放在这里实现
  return uv_run(loop_, UV_RUN_DEFAULT);
}
