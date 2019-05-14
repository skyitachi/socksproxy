//
// Created by skyitachi on 2019-05-10.
//

#ifndef SOCKSPROXY_HTTPHEADER_H
#define SOCKSPROXY_HTTPHEADER_H

#include "BytesBuffer.h"
#include <map>
#include <set>
#include <utility>

#define HTTP_HEADER_MAX_LENGTH 8192

namespace socks {
  typedef std::set<std::string> StringSet;
  
  static StringSet gMethodSet = StringSet{
    "GET",
    "POST",
    "PUT",
    "DELETE",
    "OPTIONS"
  };
  
  class HttpHeader : public bb::ByteBuffer {
  public:
    
    HttpHeader(): bb::ByteBuffer(1024) {}
    bool IsHttp(char *buf, size_t len);
    std::pair<bool, bool> receiveBytes(char *buf, size_t len);
  private:
    std::map<std::string, std::string> headers;
  };
}


#endif //SOCKSPROXY_HTTPHEADER_H
