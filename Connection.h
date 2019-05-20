//
// Created by skyitachi on 2019-04-23.
//

#ifndef SOCKSPROXY_CONNECTION_H
#define SOCKSPROXY_CONNECTION_H


#include <cstdio>
#include <list>
#include <uv.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <chrono>
#include <boost/log/trivial.hpp>
#include "util.h"
#include "HttpHeader.h"
#include "http-parser/http_parser.h"

#define SOCKS4A_HEADER_LENGTH 9
// Connection should be EventListener
namespace socks {
static int gCounter = 0;

class Connection {
public:
  typedef std::chrono::time_point<std::chrono::system_clock> SystemClock;
  enum Status {
    INIT,
    CLIENT_REQUEST,
    CONNECTED,
    SERVER_CONNECT_ERROR,
    DATA_PENDING,
    DATA_TO_WRITE,
    SERVER_CLOSE,
    SERVER_FREED,
  };
  enum Protocol {
    UNPARSED,
    TCP,
    HTTP,
    HTTP_HEADERS_COMPLETE
  };
  
  Connection() {
    initHandle<uv_tcp_t>(&tcp_);
  
    initHandle<uv_tcp_t>(&remoteTcp);
  
    initHandle<uv_connect_t>(&connectReq);
  
    initHandle<uv_timer_t>(&timer_);

    loop_ = uv_default_loop();

    uv_tcp_init(loop_, tcp_);
    uv_tcp_init(loop_, remoteTcp);
    id_ = gCounter++;
    initTimer();
    initHttpParser();
    
    protocol = UNPARSED;
    
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

  uv_connect_t* connectReq;
  // proxy to upstream connection
  uv_tcp_t* remoteTcp;

  Status status = INIT;

  util::Addr remoteAddr;

  void onData(char* receiveBuf, size_t len);
  void writeToClient(char* buf, size_t len);
  void writeToProxy(char* buf, size_t len);
  void connectToRemote(const char* ip, uint16_t port);

  void freeRemoteTcp();
  void freeTcp();
  
  void checkReadTimer(SystemClock now);
  
  Protocol protocol;
  
  http_parser* httpParserPtr;
  SystemClock lastReadTime = std::chrono::system_clock::now();
  
private:
  template<typename T>
  void initHandle(T **handle) {
    *handle = (T*)malloc(sizeof(T));
    // set context
    (*handle)->data = this;
  }
  void write(char *buf, size_t, uv_stream_t*, uv_write_cb);
  void initTimer();
  void initHttpParser();
  void parseBytes(char* buf, size_t len);
  
  // client to proxy
  uv_tcp_t* tcp_;
  uv_timer_t* timer_;

  uv_loop_t* loop_;
  int id_;
};

};
#endif //SOCKSPROXY_CONNECTION_H
