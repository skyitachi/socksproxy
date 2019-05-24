//
// Created by skyitachi on 2019-04-23.
//

#include "Connection.h"

namespace socks {
  
static int on_headers_complete(http_parser* parser);
static int on_message_begin(http_parser*);

static void on_uv_close(uv_handle_t* handle) {
  // NOTE: must free here
  free(handle);
}

static http_parser_settings httpParserSettings = {
  .on_message_begin = on_message_begin
  ,.on_url = 0
  ,.on_status = 0
  ,.on_header_field = 0
  ,.on_header_value = 0
  ,.on_headers_complete = on_headers_complete
  ,.on_body = 0
  ,.on_message_complete = 0
  ,.on_chunk_header = 0
  ,.on_chunk_complete = 0
};

static char fixedHeader[8] = {0x00, 0x5a, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47};

static void on_proxy_uv_alloc(uv_handle_t* handle, size_t suggest_size, uv_buf_t* buf) {
  Connection* conn = (Connection* )handle->data;
  assert(conn);
  buf->base = conn->upstreamBuf;
  buf->len = sizeof(conn->upstreamBuf);
}

// client stream write callback
static void on_connection_write_end(uv_write_t* req, int status) {
  Connection* conn = (Connection *)req->data;
  assert(conn);
  free(req);
  if (status < 0) {
    BOOST_LOG_TRIVIAL(error) << "on_connection_write_end error: " << uv_strerror(status);
    // 这里相当于客户端主动关闭连接
    if (conn->status == Connection::CONNECTED) {
      // 需要主动关闭客户端连接
      BOOST_LOG_TRIVIAL(info) << "on_connection_write_end freeRemoteTcp";
      conn->freeRemoteTcp();
      conn->freeTimer();
      delete conn;
    }
    return;
  }
}

static void on_write_to_upstream_done(uv_write_t* req, int status) {
  Connection* conn = (Connection *)req->data;
  assert(conn);
  free(req);
  if (status < 0) {
    if (conn->status == Connection::SERVER_CLOSE) {
      BOOST_LOG_TRIVIAL(info) << "on_write_to_upstream_done freeRemoteTcp";
      conn->freeRemoteTcp();
    }
    return;
  }
  conn->status = Connection::CONNECTED;
  conn->clientOffset = 0;
}

// proxy read upstream
static void on_proxy_uv_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* uvBuf) {
  Connection* conn = (Connection*) stream->data;
  assert(conn);
  BOOST_LOG_TRIVIAL(debug) << "on_proxy_uv_read read " << nread;
  if (nread < 0) {
    BOOST_LOG_TRIVIAL(info) << "on_proxy_uv_read error " << uv_strerror(nread);
    if (conn->status == Connection::CONNECTED) {
      conn->status = Connection::SERVER_CLOSE;
      // 如果是http的话, 这里必须清理
      BOOST_LOG_TRIVIAL(info) << "freeRemoteTcp on_proxy_uv_read";
      conn->freeRemoteTcp();
    }
    return;
  }
  if (nread > 0) {
    conn->writeToClient(uvBuf->base, nread);
  }
}

// connect to upstream callback
static void on_connect_to_upstream(uv_connect_t* req, int status) {
  Connection* conn = (Connection *)req->data;
  assert(conn);
  if (status < 0) {
    BOOST_LOG_TRIVIAL(error) << "proxy connect to server error: " << uv_strerror(status);
    conn->status = Connection::SERVER_CONNECT_ERROR;
    return;
  }

  if (conn->status == Connection::DATA_PENDING) {
    conn->writeToProxy(conn->buf, conn->clientOffset);
  }
  
  BOOST_LOG_TRIVIAL(info) << "connect to remote successfully";
  
  uv_read_start(conn->upstream(), on_proxy_uv_alloc, on_proxy_uv_read);
  conn->status = Connection::CONNECTED;
  // 清除connectReq
  free(req);
}

static void on_uv_timer_cb(uv_timer_t* timer) {
  Connection* conn = (Connection* ) timer->data;
  assert(conn);
  auto now = std::chrono::system_clock::now();
  conn->checkReadTimer(now);
}

void Connection::write(char* buf, size_t len, uv_stream_t* stream, uv_write_cb cb) {
  uv_buf_t uvBuf;
  uvBuf = uv_buf_init(buf, len);
  uv_write_t* writeReq = (uv_write_t* )malloc(sizeof(uv_write_t));
  writeReq->data = this;
  uv_write(writeReq, stream, &uvBuf, 1, cb);
}

void Connection::writeToClient(char *buf, size_t len) {
  write(buf, len, stream(), on_connection_write_end);
}

// proxy writes data to upstream
void Connection::writeToProxy(char *buf, size_t len) {
  // 目前只解析http
  if (protocol == UNPARSED) {
    parseBytes(buf, len);
  } else if (protocol == HTTP) {
    parseBytes(buf, len);
  } else if (protocol == HTTP_HEADERS_COMPLETE) {
    // 这里已经是parse 完成了
  }
  
  if (status != SERVER_FREED) {
    // 只要server没被销毁，都要发送
    status = DATA_TO_WRITE;
    write(buf, len, upstream(), on_write_to_upstream_done);
  } else {
    printf("server has been freed before write\n");
  }
}

void Connection::sendHeader() {
  writeToClient(fixedHeader, sizeof(fixedHeader));
}

void Connection::connectToRemote(const char *ip, uint16_t port) {
  struct sockaddr_in sockaddrIn;
  uv_ip4_addr(ip, port, &sockaddrIn);
  uv_tcp_connect(connectReq, remoteTcp, (const struct sockaddr*)&sockaddrIn, on_connect_to_upstream);
}

void Connection::onData(char* receiveBuf, size_t len) {
  lastReadTime = std::chrono::system_clock::now();
  BOOST_LOG_TRIVIAL(info) << "receive data bytes:  " << len << ", conn status: " << status;
  if (len < SOCKS4A_HEADER_LENGTH && status == CLIENT_ACCEPTED) {
    clientOffset += len;
    return;
  }
  if (status == SERVER_FREED || status == SERVER_CLOSE) {
    BOOST_LOG_TRIVIAL(info) << "stop read from the client";
    uv_read_stop(stream());
    freeTcp();
  }
  // client connect request
  if (receiveBuf[0] == 0x04 && receiveBuf[1] == 0x01 && status == CLIENT_ACCEPTED) {
    status = CLIENT_REQUEST;
    remoteAddr = util::parseAddr(receiveBuf, 2);
    BOOST_LOG_TRIVIAL(info) << "remote address is " << remoteAddr.ip << ":" << remoteAddr.port;
  
    // start write handshake data, connected then reply header
    // https://www.openssh.com/txt/socks4.protocol
    // NOTE: 在这里connect更合适
    // 这会导致客户端误以为连接上了, 会多发数据
    sendHeader();
    
    connectToRemote(remoteAddr.ip, remoteAddr.port);

    // read username
    size_t userNameLen = strlen(receiveBuf + SOCKS4A_HEADER_LENGTH - 1);
    if (userNameLen > 0) {
      printf("user %s connecting to proxy\n", receiveBuf + SOCKS4A_HEADER_LENGTH - 1);
    }
    if (len > userNameLen + SOCKS4A_HEADER_LENGTH) {
      // extract data buf
      size_t overhead = userNameLen + SOCKS4A_HEADER_LENGTH;
      memmove(buf, buf + overhead, len - overhead);
      status = DATA_PENDING;
      clientOffset = len - overhead;
    } else {
      clientOffset = 0;
    }
  } else if (status == CONNECTED) {
    writeToProxy(receiveBuf, len);
  } else if (status == DATA_PENDING) {
    clientOffset += len;
  } else if (status == CLIENT_REQUEST) {
    // 保留在缓冲区的buffer
    clientOffset = len;
    status = DATA_PENDING;
  } else {
    BOOST_LOG_TRIVIAL(info) << "unexpected connection status " << status;
    uv_read_stop(stream());
  }
}


void Connection::freeRemoteTcp() {
  status = SERVER_FREED;
  uv_close((uv_handle_t* )remoteTcp, on_uv_close);
}

void Connection::freeTcp() {
  uv_close((uv_handle_t* )tcp_, on_uv_close);
}

void Connection::initTimer() {
  uv_timer_init(loop_, timer_);
  // 10s 检查一次
  uv_timer_start(timer_, on_uv_timer_cb, 0, 10000);
}

void Connection::checkReadTimer(SystemClock now) {
  std::chrono::duration<double> diff = now - lastReadTime;
  if (diff.count() < 10) return;
  if (status == INIT) {
    // 超时时间
    BOOST_LOG_TRIVIAL(info) << "timeout reached, cleanup by timeout";
    cleanup();
  } else if (status == CLIENT_ACCEPTED) {
    uv_read_stop(stream());
    uv_close(clientHandle(), [](uv_handle_t* handle) {
      Connection* conn = (Connection* ) handle->data;
      assert(conn);
      BOOST_LOG_TRIVIAL(info) << "connection cleanup done, cleanup by server error";
      conn->cleanup();
    });
  } else if (status == SERVER_CONNECT_ERROR) {
    uv_close(clientHandle(), [](uv_handle_t* handle) {
      Connection* conn = (Connection* ) handle->data;
      assert(conn);
      BOOST_LOG_TRIVIAL(info) << "connection cleanup done, cleanup by server connect error";
      conn->cleanup();
    });
  } else if (status == SERVER_FREED) {
    // Note: 只有在服务端先关闭连接的情况下，客户端检测到空闲连接才会主动断开
    // clear tcp
    BOOST_LOG_TRIVIAL(info) << "clear up connection resource by timer";
    freeTcp();
    freeTimer();
    // Note: destroy all
    delete this;
  }
}

void Connection::initHttpParser() {
  httpParserPtr = (http_parser* )malloc(sizeof(http_parser));
  http_parser_init(httpParserPtr, HTTP_REQUEST);
  // set context
  httpParserPtr->data = this;
}

void Connection::parseBytes(char *buf, size_t len) {
  assert(httpParserPtr);
  size_t nparsed = http_parser_execute(httpParserPtr, &httpParserSettings, buf, len);
  if (nparsed != len) {
    // 非http的request
    protocol = TCP;
  }
}

void Connection::freeTimer() {
  assert(timer_);
  uv_timer_stop(timer_);
  free(timer_);
  timer_ = nullptr;
}

static int on_headers_complete(http_parser* parser) {
  Connection* conn = (Connection* ) parser->data;
  assert(conn);
  BOOST_LOG_TRIVIAL(info) << "http protocol parsed";
  conn->protocol = Connection::HTTP_HEADERS_COMPLETE;
  return 0;
}

static int on_message_begin(http_parser* parser) {
  Connection* conn = (Connection* ) parser->data;
  assert(conn);
  conn->protocol = Connection::HTTP;
  return 0;
}

};

