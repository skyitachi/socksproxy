#include <iostream>

#include "ProxyServer.h"

int main() {
  socks::ProxyServer server;
  
  int r = server.listen(12321);
  if (r < 0) {
    printf("listened error %s\n", uv_strerror(r));
  }
  return 0;
}