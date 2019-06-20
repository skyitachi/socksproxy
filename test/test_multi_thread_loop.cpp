//
// Created by skyitachi on 2019-06-18.
//

#include "../EventLoop.h"
#include "../EventLoopThread.h"
#include "../EventLoopThreadPool.h"
#include "../TcpServer.h"

#include <boost/log/trivial.hpp>

using namespace socks;

void onLoopCreated(uv_loop_t *loop) {
  BOOST_LOG_TRIVIAL(info) << "loop created";
}

int main() {
  EventLoopThreadPool pool(uv_default_loop());
  pool.setThreadNums(2);
  
  sockaddr_in sockaddrIn;
  uv_ip4_addr("0.0.0.0", 3000, &sockaddrIn);
  
  for (int i = 0; i < 2; i++) {
    uv_loop_t *loop = pool.getNextLoop();
    uv_tcp_t* tcp = (uv_tcp_t* )malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, tcp);
    uv_connect_t* req = (uv_connect_t* )malloc(sizeof(uv_connect_t));
    uv_tcp_connect(req, tcp, (const sockaddr *) &sockaddrIn, [](uv_connect_t *request, int status) {
      BOOST_LOG_TRIVIAL(info) << "current thread id: " << std::this_thread::get_id();
      if (status < 0) {
        BOOST_LOG_TRIVIAL(error) << "connect error " << uv_strerror(status);
        return;
      }
    });
  }
  TcpServer server(uv_default_loop());
  server.setConnectionCallback([](const TcpConnectionPtr& conn) {
    BOOST_LOG_TRIVIAL(info) << "current thread id: " << std::this_thread::get_id();
    if (conn->connected()) {
      BOOST_LOG_TRIVIAL(info) << "receive connections from " << conn->getLocalAddress();
    } else {
      BOOST_LOG_TRIVIAL(info) << "connection disconnected";
    }
  });
  server.listen("0.0.0.0", 3000);
//  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

