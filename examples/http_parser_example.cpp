//
// Created by skyitachi on 2019-05-17.
//

#include "http_parser.h"
#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

int main() {
  static http_parser_settings settings_null =
      {
      .on_message_begin = 0
          ,.on_header_field = 0
          ,.on_header_value = 0
          ,.on_url = 0
          ,.on_status = 0
          ,.on_body = 0
          ,.on_headers_complete = 0
          ,.on_message_complete = 0
          ,.on_chunk_header = 0
          ,.on_chunk_complete = 0
      };
  http_parser_settings settings = {
      .on_message_begin = 0
      ,.on_header_field = 0
      ,.on_header_value = 0
      ,.on_url = 0
      ,.on_status = 0
      ,.on_body = 0
      ,.on_headers_complete = 0
      ,.on_message_complete = 0
      ,.on_chunk_header = 0
      ,.on_chunk_complete = 0
  };
  settings.on_headers_complete = [](http_parser* parser) -> int {
    printf("in the headers_complete\n");
    return 0;
  };
  
  settings.on_message_begin = [](http_parser* parser) -> int {
    printf("in the on_message_begin\n");
    return 0;
  };
  
  http_parser* httpParser = (http_parser* )malloc(sizeof(http_parser));
  http_parser_init(httpParser, HTTP_REQUEST);
  
  const char *header = "GET /ws HTTP/1.1\r\nHost";
//  std::string header = "GET /tutorials/other/top-20-mysql-best-practices/ HTTP/1.1\r\n"
//                       "Host: net.tutsplus.com\r\n"
//                       "User-Agent: Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.1.5) Gecko/20091102 Firefox/3.5.5 (.NET CLR 3.5.30729)\n"
//                       "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
//                       "Accept-Language: en-us,en;q=0.5\r\n"
//                       "Accept-Encoding: gzip,deflate\r\n"
//                       "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
//                       "Keep-Alive: 300\r\n"
//                       "Connection: keep-alive\r\n"
//                       "Cookie: PHPSESSID=r2t5uvjq435r4q7ib3vtdjq120\r\n"
//                       "Pragma: no-cache\r\n"
//                       "Cache-Control: no-cache\r\n\r\n";
  
  size_t size = strlen(header);
  auto parsed = http_parser_execute(httpParser, &settings, header, size);
  
  printf("parsed size %zd, real size %zd\n", parsed, size);
  std::cout << httpParser->http_major << "." << httpParser->http_minor << std::endl;
  
  const char *remain = ": localhost:9001\r\n\r\n";
  size = strlen(remain);
  parsed = http_parser_execute(httpParser, &settings, remain, size);
  
  printf("parsed size %zd, real size %zd\n", parsed, size);
  
}