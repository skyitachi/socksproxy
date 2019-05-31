//
// Created by skyitachi on 2019-05-28.
//
#include <uv.h>
#include "../TcpConnection.h"
#include "../TcpServer.h"
#include <boost/log/trivial.hpp>

int main() {
  socks::TcpServer server(uv_default_loop());
  server.setConnectionCallback([](const socks::TcpConnectionPtr& ptr) {
    BOOST_LOG_TRIVIAL(info) << "connection established";

  });

  server.setMessageCallback([](const socks::TcpConnectionPtr& ptr, char *buf, ssize_t len) {
    BOOST_LOG_TRIVIAL(info) << "receive message from server: " << std::string(buf, len);
    ptr->send(std::string(buf, len));
  });
  {
    using namespace socks;
    TcpConnection conn(uv_default_loop(), 1);
  }

  server.Listen("0.0.0.0", 3000);
}

