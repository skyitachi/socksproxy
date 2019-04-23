//
// Created by skyitachi on 2019-04-23.
//

#ifndef SOCKSPROXY_EVENTLISTENER_H
#define SOCKSPROXY_EVENTLISTENER_H

#include <vector>

class EventsEmitter {
public:
  typedef void (*ConnectionListener)();
  void registerConnectionListener(ConnectionListener listener) {
    cb_.push_back(listener);
  }
  void onConnection() {
    for(auto cb: cb_) {
      cb();
    }
  };
private:
  std::vector<ConnectionListener> cb_;
};


#endif //SOCKSPROXY_EVENTLISTENER_H
