//
// Created by skyitachi on 2019-04-23.
//

#ifndef SOCKSPROXY_CONNECTION_H
#define SOCKSPROXY_CONNECTION_H


#include <cstdio>
#include <list>
#include <uv.h>
#include "util.h"

#define SOCKS4A_HEADER_LENGTH 9
// Connection should be EventListener
namespace socks {
static int gCounter = 0;
class Connection {
public:
  enum Status {
    INIT,
    CLIENT_REQUEST,
    CONNECTED,
    SERVER_CONNECT_ERROR,
    DATA_PENDING,
    SERVER_CLOSE
  };
  Connection() {
    // set context
    tcp_ = (uv_tcp_t* )malloc(sizeof(uv_tcp_t));
    tcp_->data = this;
    
    remoteTcp = (uv_tcp_t* )malloc(sizeof(uv_tcp_t));
    remoteTcp->data = this;
    
    connectReq = (uv_connect_t* )malloc(sizeof(uv_connect_t));
    connectReq->data = this;
    
    loop_ = uv_default_loop();

    uv_tcp_init(loop_, tcp_);
    uv_tcp_init(loop_, remoteTcp);
    id_ = gCounter++;
  }
  
  ~Connection() {
  }
  
  inline int id() {
    return id_;
  }

  uv_stream_t *stream() {
    return (uv_stream_t *)tcp_;
  }

  // proxy to server
  uv_stream_t *upstream() {
    return (uv_stream_t *) remoteTcp;
  }

  void sendHeader();
  
  char buf[4096];
  // buf 的 offset
  size_t clientOffset = 0;
  size_t pendingLen = 0;
  
  char upstreamBuf[4096];
  // TODO: client to proxy, proxy to server share same writeReq ???
  // short lived object, don't reuse
//  uv_write_t* writeReq;

  uv_connect_t* connectReq;
  // proxy to upstream connection
  uv_tcp_t* remoteTcp;
  
  Status status = INIT;
  
  util::Addr remoteAddr;
  
  void onData(char* receiveBuf, size_t len);
  void writeToClient(char* buf, size_t len);
  void writeToProxy(char* buf, size_t len);
  void connectToRemote(const char* ip, uint16_t port);
  
  void freeRemoteTcp() {
    uv_close((uv_handle_t* )remoteTcp, NULL);
    free(remoteTcp);
  }
  void freeTcp() {
    uv_close((uv_handle_t* )tcp_, NULL);
    free(tcp_);
  }
  
private:
  void write(char *buf, size_t, uv_stream_t*, uv_write_cb);
  // client to proxy
  uv_tcp_t* tcp_;
  
  uv_loop_t* loop_;
  int id_;
};

};
#endif //SOCKSPROXY_CONNECTION_H
