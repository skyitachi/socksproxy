cmake_minimum_required(VERSION 3.12)
project(socksproxy)

include_directories(/usr/local/include)
#link_libraries(/usr/local/lib)
link_directories(/usr/local/lib)

set(CMAKE_CXX_STANDARD 14)
add_executable(socksproxy main.cpp EventListener.h ProxyServer.cpp ProxyServer.h Connection.cpp Connection.h)
target_link_libraries(socksproxy uv)
