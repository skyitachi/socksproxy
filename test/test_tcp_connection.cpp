//
// Created by skyitachi on 2019-05-28.
//
#include <uv.h>
#include "../TcpConnection.h"
#include <boost/log/trivial.hpp>
#include <unordered_map>
int gCounter = 0;

std::unordered_map<int, socks::TcpConnectionPtr> connectionMap;

static void on_uv_connection(uv_stream_t* server, int status) {
  if (status < 0) {
    return;
  }
  auto conn = std::make_shared<socks::TcpConnection>(uv_default_loop());
  connectionMap[gCounter] = conn;
  gCounter += 1;
  
  conn->setConnectionCallback([](const socks::TcpConnectionPtr& ptr) {
    BOOST_LOG_TRIVIAL(info) << "connection established";
  });
  
  conn->setMessageCallback([](const socks::TcpConnectionPtr& ptr, char* buf, ssize_t len) {
    BOOST_LOG_TRIVIAL(info) << "receive message from server: " << std::string(buf, len);
  });
  
  if (!uv_accept(server, conn->stream())) {
    conn->connectionEstablished();
    conn->readStart();
  }
  // 如果不存储conn的话，就自动析构了
}

int main() {
  uv_tcp_t server;
  uv_tcp_init(uv_default_loop(), &server);
  sockaddr_in sockaddrIn;
  uv_ip4_addr("0.0.0.0", 3000, &sockaddrIn);
  uv_tcp_bind(&server, (const sockaddr *)&sockaddrIn, 0);
  uv_listen((uv_stream_t *)&server, 1024, on_uv_connection);
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

