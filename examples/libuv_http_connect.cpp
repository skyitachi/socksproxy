//
// Created by skyitachi on 2019-05-23.
//

#include <uv.h>
#include <cstdlib>
#include <assert.h>
#include <stdio.h>

char buf[4096];
uv_tcp_t* tcpPtr;

void on_uv_connect(uv_connect_t* req, int status) {
  if (status < 0) {
    printf("connect error %s\n", uv_strerror(status));
    return;
  }
  printf("connect ok\n");
  assert(req->handle == (uv_stream_t* )tcpPtr);
  uv_close((uv_handle_t *)req->handle, NULL);
}

int main() {
  tcpPtr = (uv_tcp_t* )malloc(sizeof(uv_tcp_t));
  uv_tcp_init(uv_default_loop(), tcpPtr);
  uv_connect_t req;
  sockaddr_in sockaddrIn;
  uv_ip4_addr("127.0.0.1", 3000, &sockaddrIn);
  uv_tcp_connect(&req, tcpPtr, (const sockaddr *)&sockaddrIn, on_uv_connect);
  uv_run(uv_default_loop(), UV_RUN_ONCE);
}

