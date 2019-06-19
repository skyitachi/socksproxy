//
// Created by skyitachi on 2019-06-06.
//

#include <uv.h>
#include "../TcpConnection.h"
#include "../TcpServer.h"
#include "../TcpClient.h"
#include <boost/log/trivial.hpp>
#include <iostream>

using namespace socks;

int main() {
  TcpServer server(uv_default_loop());
  server.setMessageCallback([](const TcpConnectionPtr& ptr, char* buf, ssize_t len) {
    BOOST_LOG_TRIVIAL(info) << "server receive message: " << std::string(buf, len);
    // echo
    ptr->send(buf, len);
  });
  
  server.setConnectionCallback([](const TcpConnectionPtr& ptr) {
    if (ptr->connected()) {
      BOOST_LOG_TRIVIAL(info) << "remote " << ptr->getPeerAddress() << " has connected to " << ptr->getLocalAddress();
    } else {
      BOOST_LOG_TRIVIAL(info) << "client disconnected ";
    }
  });
  
  int ret = server.listen("0.0.0.0", 3000);
  if (ret) {
    BOOST_LOG_TRIVIAL(error) << "server listen error " << uv_strerror(ret);
  }
}

