//
// Created by skyitachi on 2019-05-31.
//

#ifndef SOCKSPROXY_TCPCLIENT_H
#define SOCKSPROXY_TCPCLIENT_H

#include <memory>
#include <uv.h>
#include <unordered_map>
#include "TcpConnection.h"
#include "util.h"

namespace socks {
  
  class TcpClient: socks::util::NoCopyable {
  public:
    TcpClient(uv_loop_t* loop): loop_(loop), tcpPtr(std::make_unique<uv_tcp_t>()) {
      tcpPtr->data = this;
      uv_tcp_init(loop_, tcpPtr.get());
    }
    
    void setConnectionCallback(ConnectionCallback cb) {
      connectionCallback = cb;
    }
    
    void setMessageCallback(MessageCallback cb) {
      messageCallback = cb;
    }
    
    void connect(const std::string& host, int port);
   
    uv_stream_t* stream() {
      return (uv_stream_t*) tcpPtr.get();
    }
    uv_loop_t* getLoop() {
      return loop_;
    }
  
    int getNextId() { return id_++; }
    
    ~TcpClient() {
      if (tcpPtr == nullptr) return;
      auto uv_tcp = tcpPtr.release();
      uv_close((uv_handle_t* )uv_tcp, [](uv_handle_t* handle) {
        delete handle;
      });
    }
    
    ConnectionCallback connectionCallback;
    MessageCallback messageCallback;
    
    std::unique_ptr<uv_tcp_t> tcpPtr;
    std::unordered_map<int, TcpConnectionPtr> connectionMap;
    
  private:
    uv_loop_t* loop_;
    int id_;
  };
}


#endif //SOCKSPROXY_TCPCLIENT_H
