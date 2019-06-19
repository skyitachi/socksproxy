//
// Created by skyitachi on 2019-06-18.
//

#include "EventLoopThread.h"

namespace socks {
  
  EventLoopThread::EventLoopThread(const socks::EventLoopThread::ThreadInitCallback &cb):
    cb_(cb),
    thread_(std::thread(std::bind(&EventLoopThread::threadFunc, this))) {
    loop_ = uv_loop_new();
  }
  
  void EventLoopThread::threadFunc() {
    cb_(loop_);
  }
  
  void EventLoopThread::startLoop() {
    uv_run(loop_, UV_RUN_DEFAULT);
  }
  
  EventLoopThread::~EventLoopThread() {
    uv_stop(loop_);
    thread_.join();
  }
  
}