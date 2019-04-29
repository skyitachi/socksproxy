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
  Connection(uv_loop_t *l): loop_(l) {}
  size_t write();
  void addDataListener();
private:
  void onData_();
  uv_loop_t* loop_;
};


#endif //SOCKSPROXY_CONNECTION_H
