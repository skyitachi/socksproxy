//
// Created by skyitachi on 2019-06-18.
//

#include "EventLoopThread.h"
#include <boost/log/trivial.hpp>

namespace socks {
  
  EventLoopThread::EventLoopThread():
    loop_(uv_loop_new()),
    thread_(std::thread(std::bind(&EventLoopThread::threadFunc, this))) {
  }
  
  EventLoopThread::EventLoopThread(const socks::EventLoopThread::ThreadInitCallback &cb):
    cb_(cb),
    loop_(uv_loop_new()),
    thread_(std::thread(std::bind(&EventLoopThread::threadFunc, this))) {
  }
  
  void EventLoopThread::threadFunc() {
    assert(loop_);
    if (cb_ != nullptr) {
      cb_(loop_);
    }
    started_ = true;
    condition_.notify_one();
    // busy wait
    while(true) {
      int r = uv_run(loop_, UV_RUN_DEFAULT);
      if (r > 0) {
        BOOST_LOG_TRIVIAL(info) << "loop stoped has still active";
        break;
      } else if (r < 0) {
        BOOST_LOG_TRIVIAL(error) << "loop error " << uv_strerror(r);
        break;
      }
    }
    BOOST_LOG_TRIVIAL(trace) << "after the threadFunc";
    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = nullptr;
  }
  
  uv_loop_t * EventLoopThread::startLoop() {
    std::unique_lock<std::mutex> lk(mutex_);
    condition_.wait(lk, [this]() { return started_; });
    lk.unlock();
    return loop_;
  }
  
  EventLoopThread::~EventLoopThread() {
    BOOST_LOG_TRIVIAL(trace) << "EventLoopThread destructor called";
    uv_stop(loop_);
    thread_.join();
  }
  
}