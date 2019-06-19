//
// Created by skyitachi on 2019-06-18.
//

#include "../EventLoop.h"
#include "../EventLoopThread.h"
#include <boost/log/trivial.hpp>

using namespace socks;

void onLoopCreated(uv_loop_t *loop) {
  BOOST_LOG_TRIVIAL(info) << "loop created";
}

int main() {
  EventLoopThread loopThread(onLoopCreated);
  loopThread.startLoop();
}

