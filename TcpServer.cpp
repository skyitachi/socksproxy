//
// Created by skyitachi on 2019/5/28.
//

#include "TcpServer.h"

namespace socks {

  using namespace std::placeholders;
  
  static void closeConnection(TcpServer* server, const TcpConnectionPtr& conn) {
    BOOST_LOG_TRIVIAL(info) << "delete conn " << conn->id() << " from connectionMap";
    assert(server->connectionMap_.find(conn->id()) != server->connectionMap_.end());
    server->connectionMap_.erase(conn->id());
  }
  
  static void on_uv_connection(uv_stream_t* server, int status) {
    auto tcpServer = (TcpServer*) server->data;
    assert(tcpServer);
    if (status < 0) {
      // TODO: 错误处理
      return;
    }
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(tcpServer->getLoop(), tcpServer->getNextId());
    tcpServer->addTcpConnection(conn);
    
    if (!uv_accept(server, conn->stream())) {
      conn->setMessageCallback(tcpServer->messageCallback);
      conn->setConnectionCallback(tcpServer->connectionCallback);
      conn->connectionEstablished();
      conn->setCloseCallback(std::bind(closeConnection, tcpServer, _1));
      conn->readStart();
    }

  }

  int TcpServer::Listen(const std::string host, int port) {
    sockaddr_in sockaddrIn;
    uv_ip4_addr(host.c_str(), port, &sockaddrIn);
    // TODO 错误处理
    uv_tcp_bind(tcp_.get(), (const sockaddr*)&sockaddrIn, 0);
    uv_listen(stream(), 1024, on_uv_connection);
    return uv_run(loop_, UV_RUN_DEFAULT);
  }
}
