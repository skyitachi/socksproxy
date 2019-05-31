//
// Created by skyitachi on 2019-05-31.
//

#ifndef SOCKSPROXY_TCPCLIENT_H
#define SOCKSPROXY_TCPCLIENT_H

#include <memory>
#include <uv.h>
#include "util.h"

namespace socks {
  
  class TcpClient: socks::util::NoCopyable {
  public:
    TcpClient(uv_loop_t* loop): loop_(loop), tcp_(std::make_unique<uv_tcp_t>()) {
      uv_tcp_init(loop_, tcp_.get());
    }
  private:
    uv_loop_t* loop_;
    std::unique_ptr<uv_tcp_t> tcp_;
  };
}


#endif //SOCKSPROXY_TCPCLIENT_H
