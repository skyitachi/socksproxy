//
// Created by skyitachi on 2019-04-23.
//

#include "Connection.h"

static char fixedHeader[8] = {0x00, 0x5a, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47};

static void on_connection_write_end(uv_write_t* req, int status) {

}

void Connection::write(char* buf, size_t len) {
  uv_buf_t uvBuf;
  uvBuf = uv_buf_init(buf, len);
  uv_write(writeReq, stream(), &uvBuf, 1, on_connection_write_end);
}

void Connection::sendHeader() {
  write(fixedHeader, sizeof(fixedHeader));
}
