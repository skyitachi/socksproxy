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
    } else if (nread == 0) {
      BOOST_LOG_TRIVIAL(debug) << conn->id() << " read 0 bytes";
      // TODO: read = 0 意味着什么
      conn->close();
    } else {
      conn->handleMessage(buf->base, nread);
    }
  }
  
  int TcpConnection::readStart() {
    return uv_read_start(stream(), on_uv_alloc, on_uv_read);
  }
  
  int TcpConnection::readStop() {
    BOOST_LOG_TRIVIAL(debug) << "conn " << id() << " stop read";
    return uv_read_stop(stream());
  }
  
  int TcpConnection::send(const char *sendBuf, size_t len) {
    // write_req 这类的对象不需要传递data，使用handle->data即可
    assert(state_ == kConnected);
    lastWrite_ = len;
    uv_buf_t uv_buf = uv_buf_init(const_cast<char *>(sendBuf), len);
    
    // NOTE: 如果使用uv_try_write就需要自己管理output buffer了, try_write不会queue write request
    return uv_write(new uv_write_t, stream(), &uv_buf, 1, [](uv_write_t* req, int status) {
      // 留给unique_ptr自动管理
      std::unique_ptr<uv_write_t> reqHolder(req);
      auto conn = (TcpConnection *)req->handle->data;
      assert(conn);
      if (status < 0) {
        BOOST_LOG_TRIVIAL(error) << "connection write error " << uv_strerror(status);
        conn->handleClose();
        return;
      }
      conn->handleWrite(conn->lastWrite());
      conn->resetLastWrite();
    });
  }
  
  // stop write
  int TcpConnection::shutdown() {
    return uv_shutdown(new uv_shutdown_t, stream(), [](uv_shutdown_t* req, int status) {
      std::unique_ptr<uv_shutdown_t> reqHolder(req);
      auto conn = (TcpConnection *)req->handle->data;
      assert(conn);
      if (status < 0) {
        // TODO: 错误处理
        return;
      }
      BOOST_LOG_TRIVIAL(info) << "connection " << conn->id() << " shutdown write";
    });
  }
  
  // when read 0 size then close
  void TcpConnection::close() {
    handleClose();
  }
  
  void TcpConnection::handleClose() {
    assert(state_ == kConnected || state_ == kConnecting);
    // TODO: 是否要判断当前的write是否被写完
    setState(kDisconnected);
    readStop();
    connectionCallback_(shared_from_this());
    closeCallback_(shared_from_this());
  }
}
