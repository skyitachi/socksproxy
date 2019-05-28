//
// Created by skyitachi on 2019-05-27.
//

#ifndef SOCKSPROXY_TCPCONNECTION_H
#define SOCKSPROXY_TCPCONNECTION_H

#include <uv.h>
#include <memory>
#include <functional>

namespace socks {
class TcpConnection: public std::enable_shared_from_this<TcpConnection> {
  public:
  
    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
    typedef std::function<void (const TcpConnectionPtr& )> ConnectionCallback;
    typedef std::function<void (const TcpConnectionPtr&, char *buf, ssize_t len)> MessageCallback;
    
    enum StateE {
      kConnecting, kConnected, kDisconnectd, kDisconnecting
    };
    
    TcpConnection(uv_loop_t *loop) : loop_(loop), tcp_(std::make_unique<uv_tcp_t>()) {
      tcp_->data = this;
      uv_tcp_init(loop_, tcp_.get());
    }
    
    void setConnectionCallback(ConnectionCallback cb) {
      connectionCallback_ = cb;
    }
    
    void setMessageCallback(MessageCallback cb) {
      messageCallback_ = cb;
    }
    
    uv_stream_t* stream() {
      return (uv_stream_t *) tcp_.get();
    }
    
    void connectionEstablished() {
      connectionCallback_(shared_from_this());
    }
    
    void readStart();
    
    void handleMessage(char* buf, ssize_t len) {
      messageCallback_(shared_from_this(), buf, len);
    }
    
    ~TcpConnection() {
      auto raw = tcp_.release();
      uv_close((uv_handle_t *) raw, NULL);
    }
    
    char buf[4096];
  
  private:
    uv_loop_t *loop_;
    std::unique_ptr<uv_tcp_t> tcp_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
  };
  
  typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
}
#endif //SOCKSPROXY_TCPCONNECTION_H
