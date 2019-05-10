//
// Created by skyitachi on 2019-05-10.
//

#ifndef SOCKSPROXY_HTTPHEADER_H
#define SOCKSPROXY_HTTPHEADER_H

#include "BytesBuffer.h"
#include <map>

class HttpHeader: public bb::ByteBuffer {
private:
  std::map<std::string, std::string> headers;
};


#endif //SOCKSPROXY_HTTPHEADER_H
