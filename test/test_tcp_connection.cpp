//
// Created by skyitachi on 2019-05-28.
//
#include <uv.h>
#include "../TcpConnection.h"
#include "../TcpServer.h"
#include "../TcpClient.h"
#include <boost/log/trivial.hpp>
#include <thread>
#include <iostream>

using namespace socks;

void onConnection(const TcpConnectionPtr& ptr) {
  if (ptr->connected()) {
    BOOST_LOG_TRIVIAL(info) << "connect to remote successfully";
  } else if (ptr->disconnected()) {
    BOOST_LOG_TRIVIAL(info) << "disconnect to remote successfully";
  }
}

int main() {
//  socks::TcpServer server(uv_default_loop());
//  server.setConnectionCallback([](const socks::TcpConnectionPtr& ptr) {
//    BOOST_LOG_TRIVIAL(info) << "connection established";
//  });
//
//  server.setMessageCallback([](const socks::TcpConnectionPtr& ptr, char *buf, ssize_t len) {
//    BOOST_LOG_TRIVIAL(info) << "receive message from server: " << std::string(buf, len);
//    ptr->send(std::string(buf, len));
//  });
//  server.Listen("0.0.0.0", 3000);
  {
    TcpClient tcpClient(uv_default_loop());
    tcpClient.setConnectionCallback([&tcpClient](const TcpConnectionPtr& ptr) {
      if (ptr->connected()) {
        ptr->send("hello");
        BOOST_LOG_TRIVIAL(info) << "connect to remote successfully";
      } else if (ptr->disconnected()) {
        BOOST_LOG_TRIVIAL(info) << "disconnect to remote successfully";
      }
    });
    
    tcpClient.setMessageCallback([](const TcpConnectionPtr& ptr, char *buf, ssize_t len) {
      BOOST_LOG_TRIVIAL(info) << "receive message from server: " << std::string(buf, len);
    });
    
    tcpClient.setWriteCompleteCallback([](const TcpConnectionPtr& ptr) {
      BOOST_LOG_TRIVIAL(info) << ptr->id() << " has written " << ptr->bytesWritten() << " bytes data";
    });
    
    tcpClient.connect("localhost", 3000);
    
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  }
  
  std::cout << "exit to the main\n";

}

