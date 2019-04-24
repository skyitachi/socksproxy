//
// Created by skyitachi on 2019-04-23.
//

#ifndef SOCKSPROXY_CONNECTION_H
#define SOCKSPROXY_CONNECTION_H


#include <cstdio>
#include <list>

// Connection should be EventListener
class Connection {
public:
  size_t write();
  void addDataListener();
private:
  void onData_();
};


#endif //SOCKSPROXY_CONNECTION_H
