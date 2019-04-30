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
    loop_ = uv_default_loop();
    uv_tcp_init(loop_, tcp_);
  }
  ~Connection() {
    uv_close((uv_handle_t *)tcp_, NULL);
    free(tcp_);
  }

  uv_stream_t *stream() {
    return (uv_stream_t *)tcp_;
  }
  size_t write();
  void addDataListener();
  char buf[4096];
private:
  void onData_();
  uv_tcp_t* tcp_;
  uv_loop_t* loop_;
};


#endif //SOCKSPROXY_CONNECTION_H
