//
// Created by skyitachi on 2019-04-23.
//

#include "ProxyServer.h"

namespace socks {

static void on_uv_client_alloc(uv_handle_t *handle, size_t suggest_size, uv_buf_t *buf) {
  Connection* conn = (Connection* )handle->data;
  assert(conn);
  buf->base = conn->buf + conn->clientOffset;
  buf->len = sizeof(conn->buf) - conn->clientOffset;
  if (buf->len < 0) {
     buf->len = 0;
  }
}

// proxy read client data
static void on_uv_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* uvBuf) {
  Connection* conn = (Connection *)stream->data;
  assert(conn);
  if (nread < 0) {
    if (nread != UV_EOF) {
      BOOST_LOG_TRIVIAL(error) << "read error: " << uv_strerror(nread);
    } else {
      // clear source
//      conn->freeTcp();
//      delete conn;
    }
    return;
  }
  conn->onData(conn->buf, nread + conn->clientOffset);
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
    BOOST_LOG_TRIVIAL(error) << "on_new_connection error: " << uv_strerror(status);
    return;
  }
  ProxyServer* serverCtx = (ProxyServer* ) server->data;
  assert(serverCtx);
  // create connection here
  Connection* conn = new Connection();
  BOOST_LOG_TRIVIAL(info) << "receive connect request conn->id " << conn->id();
  if (!uv_accept(server, conn->stream())) {
    // connection accepted
    BOOST_LOG_TRIVIAL(info) << "client connection accepted";
    conn->status = Connection::CLIENT_ACCEPTED;
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
  BOOST_LOG_TRIVIAL(info) << "listened ok";
  // TODO: 不应该放在这里实现
  return uv_run(loop_, UV_RUN_DEFAULT);
}
};