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
    EventLoopThread();
    EventLoopThread(const ThreadInitCallback&);
    ~EventLoopThread();
    uv_loop_t * startLoop();
  private:
    // 顺序会影响构造函数的写法
    uv_loop_t* loop_;
    void threadFunc();
    const ThreadInitCallback cb_;
    std::thread thread_;
    std::mutex mutex_;
    bool started_;
    std::condition_variable condition_;
  };
}


#endif //SOCKSPROXY_EVENTLOOPTHREAD_H
