//
// Created by skyitachi on 2019-05-31.
//

#include "TcpClient.h"

namespace socks {
  using namespace std::placeholders;
  
  static void closeConnection(TcpClient* tcpClient, const TcpConnectionPtr& conn) {
    BOOST_LOG_TRIVIAL(debug) << "delete conn " << conn->id();
    // NOTE: 不能调用conn->close()
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
      tcpClient->connection = std::make_shared<TcpConnection>(
        tcpClient->getLoop(),
        std::move(tcpClient->tcpPtr),
        tcpClient->getNextId()
      );
      if (tcpClient->connectionCallback) {
        tcpClient->connection->setConnectionCallback(tcpClient->connectionCallback);
      }
      if (tcpClient->messageCallback) {
        tcpClient->connection->setMessageCallback(tcpClient->messageCallback);
      }
      if (tcpClient->writeCompleteCallback) {
        tcpClient->connection->setWriteCompleteCallback(tcpClient->writeCompleteCallback);
      }
      tcpClient->connection->setCloseCallback(std::bind(closeConnection, tcpClient, _1));
      
      tcpClient->connection->connectionEstablished();
      tcpClient->connection->readStart();
    });
  }
  
  void TcpClient::disconnect() {
    assert(connection != nullptr && connection->connected());
    connection->shutdown();
  }
}
