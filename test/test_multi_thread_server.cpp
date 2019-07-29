//
// Created by skyitachi on 2019-06-20.
//

#include "../TcpServer.h"
#include "../TcpConnection.h"
#include <boost/log/trivial.hpp>

using namespace socks;

int main() {
  TcpServer server(uv_default_loop());
  server.setThreadNums(4);
  
  server.setMessageCallback([](const TcpConnectionPtr& ptr, char* buf, ssize_t len) {
    BOOST_LOG_TRIVIAL(info) << "current thread id " << std::this_thread::get_id();
    BOOST_LOG_TRIVIAL(info) << "server receive message: " << std::string(buf, len);
    // echo
    ptr->send(buf, len);
  });
  
  server.setConnectionCallback([](const TcpConnectionPtr& ptr) {
    BOOST_LOG_TRIVIAL(info) << "current thread id " << std::this_thread::get_id();
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
