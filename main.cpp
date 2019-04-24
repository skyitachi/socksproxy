#include <iostream>

#include "EventListener.h"
#include "ProxyServer.h"

void testCb() {
  std::cout << "in the testCb callback" << std::endl;
}

int main() {
  std::cout << "Hello, World!" << std::endl;
  EventsEmitter emitter;
  emitter.registerConnectionListener(testCb);
  emitter.onConnection();
  
  ProxyServer server;
  
  int r = server.listen(12321);
  if (r < 0) {
    printf("listened error %s\n", uv_strerror(r));
  }
  return 0;
}