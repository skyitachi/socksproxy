//
// Created by skyitachi on 2019-04-23.
//

#ifndef SOCKSPROXY_CONNECTION_H
#define SOCKSPROXY_CONNECTION_H


#include <cstdio>
#include <list>
#include <uv.h>

// Connection should be EventListener
class Connection {
public:
  Connection() {
    tcp_ = (uv_tcp_t* )malloc(sizeof(uv_tcp_t));
    // set context
    tcp_->data = this;
    
    remoteTcp = (uv_tcp_t* )malloc(sizeof(uv_tcp_t));
    // set context
    remoteTcp->data = this;
    
    pToSProxy_ = (uv_tcp_t* )malloc(sizeof(uv_tcp_t));
    pToSProxy_->data = this;
    writeReq = (uv_write_t* )malloc(sizeof(uv_write_t));
    writeReq->data = this;
    connectReq = (uv_connect_t* )malloc(sizeof(uv_connect_t));
    connectReq->data = this;
    loop_ = uv_default_loop();
    uv_tcp_init(loop_, tcp_);
    uv_tcp_init(loop_, pToSProxy_);
  }
  ~Connection() {
    uv_close((uv_handle_t *)tcp_, NULL);
    free(tcp_);
  }

  uv_stream_t *stream() {
    return (uv_stream_t *)tcp_;
  }
  
  void write(char *buf, size_t len);
  void sendHeader();
  void addDataListener();
  char buf[4096];
  uv_write_t* writeReq;
  uv_connect_t* connectReq;
  uv_tcp_t* remoteTcp;
private:
  void onData_();
  // client to proxy
  uv_tcp_t* tcp_;
  // proxy to server
  
  // proxy to server;
  uv_tcp_t* pToSProxy_;
  uv_loop_t* loop_;
};


#endif //SOCKSPROXY_CONNECTION_H
