//
// Created by skyitachi on 2019-04-23.
//

#include "ProxyServer.h"

void ProxyServer::onConnection() {

}

void ProxyServer::addConnectionListener(ProxyServer::ConnectionListener listener) {
  listener_.push_back(listener);
}
