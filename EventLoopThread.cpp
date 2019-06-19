//
// Created by skyitachi on 2019-06-18.
//

#include "EventLoopThread.h"

namespace socks {
  
  EventLoopThread::EventLoopThread(const socks::EventLoopThread::ThreadInitCallback &cb):
    cb_(cb),
    loop_(uv_loop_new()), // 保证 loop 已经创建过
    thread_(std::thread(std::bind(&EventLoopThread::threadFunc, this))) {
  }
  
  void EventLoopThread::threadFunc() {
    assert(loop_);
    cb_(loop_);
    uv_run(loop_, UV_RUN_DEFAULT);
    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = nullptr;
  }
  
  uv_loop_t * EventLoopThread::startLoop() {
    std::lock_guard<std::mutex> lock(mutex_);
    return loop_;
  }
  
  EventLoopThread::~EventLoopThread() {
    uv_stop(loop_);
    thread_.join();
  }
  
}