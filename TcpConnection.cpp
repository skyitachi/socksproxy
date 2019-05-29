//
// Created by skyitachi on 2019-05-27.
//

#include "TcpConnection.h"

namespace socks {
  static void on_uv_alloc(uv_handle_t* handle, size_t suggest_size, uv_buf_t* buf) {
    auto conn = (TcpConnection* ) handle->data;
    assert(conn);
    buf->len = sizeof(conn->buf);
    buf->base = conn->buf;
  }
  
  static void on_uv_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    auto conn = (TcpConnection* ) stream->data;
    assert(conn);
    if (nread < 0) {
      BOOST_LOG_TRIVIAL(info) << "close connection " << conn->id() << " passively";
      conn->handleClose();
    } else {
      conn->handleMessage(buf->base, nread);
    }
  }
  
  void TcpConnection::readStart() {
    uv_read_start(stream(), on_uv_alloc, on_uv_read);
  }
}
