set(CMAKE_CXX_STANDARD 14)
# 使用cmake dynamic link
set(CMAKE_CXX_FLAGS "-g -O0 -DBOOST_LOG_DYN_LINK")

# include_directories(../deps/libuv/include)

add_executable(test test.cpp)
target_link_libraries(test gtest gmock)

# link_directories(../deps/libuv)

add_executable(test_tcp_client test_tcp_connection.cpp ../TcpConnection.cpp ../TcpClient.cpp)
add_executable(test_tcp_server test_tcp_server.cpp ../TcpConnection.cpp ../TcpServer.cpp)

IF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    target_link_libraries(test_tcp_client uv boost_log-mt)
    target_link_libraries(test_tcp_server uv boost_log-mt)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(test_tcp_client uv boost_log pthread)
ENDIF ()

