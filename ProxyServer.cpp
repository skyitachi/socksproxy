//
// Created by skyitachi on 2019-04-23.
//

#include "ProxyServer.h"

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

static void on_uv_alloc(uv_handle_t* handle, size_t suggest_size, uv_buf_t* buf) {
  Connection* conn = (Connection* )handle->data;
  buf->base = conn->buf;
  buf->len = sizeof(conn->buf);
}

static void on_uv_connect(uv_connect_t* req, int status) {
  if (status < 0) {
    printf("connect error: %s\n", uv_strerror(status));
    return;
  }
  Connection *conn = (Connection* )req->data;
  assert(conn);
  
}

static void on_uv_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* uvBuf) {
  if (nread < 0) {
    // TODO: error happens
    
  } else if (nread < SOCKS4A_HEADER_LENGTH) {
    return;
  }
  Connection* conn = (Connection *)stream->data;
  assert(conn);
  // socks4a proxy
  if (uvBuf->base[0] == 0x04 && uvBuf->base[1] == 0x01) {
    printf("client request received\n");
    // TODO: parse remote address
    Addr *addr = parseAddr(uvBuf->base, 2);
    struct sockaddr_in dest;
    uv_ip4_addr(addr->ip, addr->port, &dest);
    uv_tcp_connect(conn->connectReq, conn->remoteTcp, (const struct sockaddr* )&dest, on_uv_connect);
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
    uv_read_start(conn->stream(), on_uv_alloc, on_uv_read);
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
  // TODO: 不应该放在这里实现
  return uv_run(loop_, UV_RUN_DEFAULT);
}
