//
// Created by skyitachi on 2019-04-23.
//

#ifndef SOCKSPROXY_PROXYSERVER_H
#define SOCKSPROXY_PROXYSERVER_H


#include <cstdint>
#include <list>
#include <unordered_map>
#include <uv.h>

class Connection;

// TODO: 先不考虑protocol, 暂且当成socks4a的proxy实现
class ProxyServer {
public:
  typedef void (*ConnectionListener)(Connection*, Connection*);
  ProxyServer() {
    loop_ = uv_default_loop();
  }
  ProxyServer(uv_loop_t* l): loop_(l) { }
  uv_loop_t* loop() const {
    return loop_;
  }
  int listen(uint16_t port);
  void addConnectionListener(ConnectionListener);
private:
  void onConnection_(); // proxy connection callback
  static void on_uv_connection(uv_stream_t*, int); // libuv_callbacks
  std::list<ConnectionListener> listener_;
  std::unordered_map<Connection*, Connection*> pipe_;
  uv_loop_t* loop_;
  uv_tcp_t server_;
};


#endif //SOCKSPROXY_PROXYSERVER_H
