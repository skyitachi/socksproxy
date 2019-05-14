//
// Created by skyitachi on 2019-05-10.
//

#include "HttpHeader.h"
#include <string.h>
namespace socks {
  // return parse status and is http protocol
  std::pair<bool, bool> HttpHeader::receiveBytes(char *input_buf, size_t len) {
    putBytes((uint8_t* )input_buf, (uint32_t) len);
    if (size() == HTTP_HEADER_MAX_LENGTH) {
      clear();
      return std::make_pair(true, false);
    }
    uint32_t remaining = bytesRemaining();
    bool foundCRLF = false;
    uint32_t lastSpace = -1;
    while (remaining > 1) {
      uint8_t c = get();
      uint8_t next = get();
      if (c == 0x0d && next == 0x0a) {
        foundCRLF = true;
        break;
      }
      if (c == 0x20) {
        lastSpace = getReadPos() - 1;
      } else if (next == 0x20) {
        lastSpace = getReadPos();
      }
      remaining -= 2;
    }
    if (foundCRLF && lastSpace != -1) {
      uint32_t rPos = getReadPos() - 1;
      auto st = buf.begin() + lastSpace;
      auto end = buf.begin() + rPos;
      std::string versionStr(st, end);
      printf("http version is %s\n", versionStr.c_str());
      return std::make_pair(true, true);
    }
    return std::make_pair(false, false);
  }
}
