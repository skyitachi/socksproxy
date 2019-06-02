//
// Created by skyitachi on 2019-05-27.
//

#ifndef SOCKSPROXY_TCPCONNECTION_H
#define SOCKSPROXY_TCPCONNECTION_H

#include <uv.h>
#include <memory>
#include <functional>
#include <boost/log/trivial.hpp>
#include "util.h"

namespace socks {
class TcpConnection:
    socks::util::NoCopyable,
    public std::enable_shared_from_this<TcpConnection> {
  public:
  
    typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
    typedef std::function<void (const TcpConnectionPtr& )> ConnectionCallback;
    typedef std::function<void (const TcpConnectionPtr&, char *buf, ssize_t len)> MessageCallback;
    typedef std::function<void (const TcpConnectionPtr&) > CloseCallback;
    
    enum StateE {
      kConnecting, kConnected, kDisconnectd, kDisconnecting
    };
    
    TcpConnection(uv_loop_t *loop, int id) : loop_(loop), id_(id), tcp_(std::make_unique<uv_tcp_t>()) {
      tcp_->data = this;
      uv_tcp_init(loop_, tcp_.get());
      setState(kConnecting);
    }
    
    explicit TcpConnection(uv_loop_t* loop): TcpConnection(loop, 0) {}
    
    TcpConnection(uv_loop_t* loop, std::unique_ptr<uv_tcp_t>&& ptr, int id): TcpConnection(loop, id) {
      // NOTE: 需要用move
      tcp_ = std::move(ptr);
    }
    
    TcpConnection(uv_loop_t* loop, std::unique_ptr<uv_tcp_t>&& ptr): TcpConnection(loop, std::move(ptr), 0) {}
  
    void setCloseCallback(CloseCallback cb) {
      closeCallback_ = cb;
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
      setState(kConnected);
      connectionCallback_(shared_from_this());
    }
  
    // shutdown
    int shutdown();
    // 主动关闭链接
    void close();
    int readStart();
    int readStop();
    
    void handleMessage(char* buf, ssize_t len) {
      messageCallback_(shared_from_this(), buf, len);
    }
    
    void handleClose();
    
    ~TcpConnection() {
      BOOST_LOG_TRIVIAL(info) << "in the TcpConnection destructor";
      // tcp_所有权已经被转移
      if (tcp_ == nullptr) return;
      auto raw = tcp_.release();
      uv_close((uv_handle_t *) raw, [](uv_handle_t* handle) {
        delete handle;
      });
    }
    
    int id() {
      return id_;
    }
    
    char buf[4096];
 
    void setState(StateE state) {
      state_ = state;
    }
    
    int send(const char *, size_t);
    int send(const std::string& data) {
      return send(data.c_str(), (size_t)data.size());
    }
    
  private:
    uv_loop_t *loop_;
    std::unique_ptr<uv_tcp_t> tcp_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
    int id_;
    StateE state_;
  };
  
  typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
  typedef std::function<void (const TcpConnectionPtr& )> ConnectionCallback;
  typedef std::function<void (const TcpConnectionPtr&, char *buf, ssize_t len)> MessageCallback;
  typedef std::function<void (const TcpConnectionPtr&) > CloseCallback;
}
#endif //SOCKSPROXY_TCPCONNECTION_H
