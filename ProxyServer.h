//
// Created by skyitachi on 2019-04-23.
//

#ifndef SOCKSPROXY_PROXYSERVER_H
#define SOCKSPROXY_PROXYSERVER_H


#include <cstdint>
#include <list>
#include <uv.h>
#include <memory>
#include <assert.h>
#include "Connection.h"
#include "util.h"

namespace socks {

class ProxyServer {
public:
  typedef void (*ConnectionListener)(Connection*);
  ProxyServer() {
    loop_ = uv_default_loop();
  }
  ProxyServer(uv_loop_t* l): loop_(l) { }
  uv_loop_t* loop() const {
    return loop_;
  }
  uv_stream_t *stream() {
    return (uv_stream_t *) &server_;
  }
  int listen(uint16_t port);
  void addConnectionListener(ConnectionListener);
private:
  void onConnection_(Connection*); // proxy connection callback
  static void on_uv_connection(uv_stream_t*, int); // uv_listen callbacks
  std::list<ConnectionListener> listener_;
  uv_loop_t* loop_;
  uv_tcp_t server_;
};
};

#endif //SOCKSPROXY_PROXYSERVER_H