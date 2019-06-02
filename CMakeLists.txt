cmake_minimum_required(VERSION 3.12)
project(socksproxy)

include_directories(/usr/local/include)

include_directories(./deps/http-parser)

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
  # debug version libuv
  include_directories(./deps/libuv/include)
  link_directories(./deps/libuv)
ENDIF()

set(CMAKE_CXX_STANDARD 14)
# 使用cmake dynamic link
set(CMAKE_CXX_FLAGS "-g -O0 -DBOOST_LOG_DYN_LINK")


add_executable(socksproxy main.cpp TcpConnection.cpp EventListener.h ProxyServer.cpp ProxyServer.h Connection.cpp Connection.h BytesBuffer.cpp BytesBuffer.h HttpHeader.cpp HttpHeader.h ./deps/http-parser/http_parser.c TcpServer.cpp TcpServer.h)

IF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    link_directories(/usr/local/lib)
    target_link_libraries(socksproxy uv boost_log-mt)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(socksproxy uv boost_log pthread)
ENDIF ()

add_subdirectory(examples)
add_subdirectory(test)
