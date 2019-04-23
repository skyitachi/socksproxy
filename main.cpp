#include <iostream>

#include "EventListener.h"

void testCb() {
  std::cout << "in the testCb callback" << std::endl;
}

int main() {
  std::cout << "Hello, World!" << std::endl;
  EventsEmitter emitter;
  emitter.registerConnectionListener(testCb);
  emitter.onConnection();
  return 0;
}