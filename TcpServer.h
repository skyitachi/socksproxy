//
// Created by skyitachi on 2019/5/28.
//

#ifndef SOCKSPROXY_TCPSERVER_H
#define SOCKSPROXY_TCPSERVER_H

#include <uv.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <boost/log/trivial.hpp>
#include "TcpConnection.h"

namespace socks {
  class TcpServer {
  public:

    TcpServer(uv_loop_t* loop): loop_(loop), tcp_(std::make_unique<uv_tcp_t>()) {
      tcp_->data = this;
      uv_tcp_init(loop_, tcp_.get());
      id_ = 0;
    }

    int Listen(const std::string host, int port);

    void setMessageCallback(MessageCallback cb) {
      messageCallback = cb;
    }

    void setConnectionCallback(ConnectionCallback cb) {
      connectionCallback = cb;
    }

    int getNextId() {
      return id_++;
    }

    uv_stream_t* stream() {
      return (uv_stream_t* )tcp_.get();
    }

    uv_loop_t* getLoop() {
      return loop_;
    }

    // 如果传入(const TcpConnectionPtr&)会有什么影响
    void addTcpConnection(const TcpConnectionPtr conn) {
      connectionMap_[getNextId()] = conn;
    }

    ~TcpServer() {
      auto uv_tcp = tcp_.release();
      uv_close((uv_handle_t* )uv_tcp, [](uv_handle_t* handle) {
        free(handle);
      });
    }
    MessageCallback messageCallback;
    ConnectionCallback connectionCallback;
    std::unordered_map<int, TcpConnectionPtr> connectionMap_;

  private:
    int id_;
    uv_loop_t* loop_;
    std::unique_ptr<uv_tcp_t> tcp_;
  };

}


#endif //SOCKSPROXY_TCPSERVER_H
