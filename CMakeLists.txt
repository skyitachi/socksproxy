cmake_minimum_required(VERSION 3.12)
project(socksproxy)

include_directories(./deps)
include_directories(/usr/local/include)
#link_libraries(/usr/local/lib)
link_directories(/usr/local/lib)

set(CMAKE_CXX_STANDARD 14)
# 使用cmake dynamic link
set(CMAKE_CXX_FLAGS "-DBOOST_LOG_DYN_LINK")

add_executable(socksproxy main.cpp EventListener.h ProxyServer.cpp ProxyServer.h Connection.cpp Connection.h BytesBuffer.cpp BytesBuffer.h HttpHeader.cpp HttpHeader.h ./deps/http-parser/http_parser.c)
target_link_libraries(socksproxy uv boost_log-mt)

add_subdirectory(examples)
add_subdirectory(test)