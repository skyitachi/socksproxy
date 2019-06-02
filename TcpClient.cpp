//
// Created by skyitachi on 2019-05-31.
//

#include "TcpClient.h"

namespace socks {
  using namespace std::placeholders;
  
  static void closeConnection(TcpClient* tcpClient, const TcpConnectionPtr& conn) {
    BOOST_LOG_TRIVIAL(info) << "delete conn " << conn->id();
    assert(tcpClient->connectionMap.find(conn->id()) != tcpClient->connectionMap.end());
    tcpClient->connectionMap.erase(conn->id());
  }
  
  void TcpClient::connect(const std::string &host, int port) {
    sockaddr_in addr_in;
    uv_ip4_addr(host.c_str(), port, &addr_in);
    uv_tcp_connect(new uv_connect_t, tcpPtr.get(), (const sockaddr* )&addr_in, [](uv_connect_t* req, int status) {
      std::unique_ptr<uv_connect_t> reqHolder(req);
      if (status < 0) {
        // TODO: 错误处理
        BOOST_LOG_TRIVIAL(error) << "connection error " << uv_strerror(status);
        return;
      }
      auto tcpClient = (TcpClient *) req->handle->data;
      assert(tcpClient);
      TcpConnectionPtr conn = std::make_shared<TcpConnection>(
        tcpClient->getLoop(),
        std::move(tcpClient->tcpPtr),
        tcpClient->getNextId()
      );
      
      tcpClient->connectionMap[conn->id()] = conn;
      BOOST_LOG_TRIVIAL(info) << conn->id() << " add to the map";
      conn->setConnectionCallback(tcpClient->connectionCallback);
      conn->setMessageCallback(tcpClient->messageCallback);
      conn->setCloseCallback(std::bind(closeConnection, tcpClient, _1));
      conn->connectionEstablished();
      conn->readStart();
    });
  }
}
