//
// Created by skyitachi on 2019-04-23.
//

#include "ProxyServer.h"

void ProxyServer::onConnection_() {

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
  printf("new connection comes");
  std::unique_ptr<Connection> cp = std::make_unique<Connection>();
  if (!uv_accept(stream(), cp->stream())) {
    // connection accepted
  }


}

int ProxyServer::listen(uint16_t port) {
  assert(loop_);
  uv_tcp_init(loop_, &server_);
  const char* host = "0.0.0.0";
  sockaddr_in addr;
  uv_ip4_addr(host, port, &addr);
  uv_tcp_bind(&server_, (const sockaddr *)&addr, 0);
  // TODO: 尝试使用std::bind
  auto f = std::bind(&ProxyServer::on_uv_connection, this);
  int r = uv_listen((uv_stream_t *)&server_, 1, f);
  if (r < 0) {
    return r;
  }
  // TODO: 不应该放在这里实现
  return uv_run(loop_, UV_RUN_DEFAULT);
}
