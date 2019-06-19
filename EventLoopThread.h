//
// Created by skyitachi on 2019-06-18.
//

#ifndef SOCKSPROXY_EVENTLOOPTHREAD_H
#define SOCKSPROXY_EVENTLOOPTHREAD_H

#include <functional>
#include <uv.h>
#include <string>
#include <thread>
#include "EventLoop.h"
#include "util.h"

namespace socks {
  class EventLoopThread: socks::util::NoCopyable {
  public:
    typedef std::function<void (uv_loop_t*)> ThreadInitCallback;
    EventLoopThread(const ThreadInitCallback&);
    ~EventLoopThread();
    uv_loop_t * startLoop();
  private:
    void threadFunc();
    const ThreadInitCallback& cb_;
    std::thread thread_;
    uv_loop_t* loop_;
    std::mutex mutex_;
  };
}


#endif //SOCKSPROXY_EVENTLOOPTHREAD_H
