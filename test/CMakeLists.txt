set(CMAKE_CXX_STANDARD 14)
# 使用cmake dynamic link
set(CMAKE_CXX_FLAGS "-g -O0 -DBOOST_LOG_DYN_LINK")

add_executable(test test.cpp)
target_link_libraries(test gtest gmock)

add_executable(test_tcp_connection test_tcp_connection.cpp ../TcpConnection.cpp ../TcpServer.cpp)
target_link_libraries(test_tcp_connection uv boost_log-mt)
