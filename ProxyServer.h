//
// Created by skyitachi on 2019-04-23.
//

#ifndef SOCKSPROXY_PROXYSERVER_H
#define SOCKSPROXY_PROXYSERVER_H


#include <cstdint>
#include <list>
#include <unordered_map>

class Connection;

// TODO: 先不考虑protocol, 暂且当成socks4a的proxy实现
class ProxyServer {
public:
  typedef void (*ConnectionListener)(Connection*, Connection*);
  int listen(uint16_t port);
  void addConnectionListener(ConnectionListener);
private:
  void onConnection();
  std::list<ConnectionListener> listener_;
  std::unordered_map<Connection*, Connection*> pipe_;
};


#endif //SOCKSPROXY_PROXYSERVER_H
