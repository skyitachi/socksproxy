include_directories(../deps/http-parser)

add_executable(http_parser_example http_parser_example.cpp ../deps/http-parser/http_parser.c)
add_executable(libuv_readfile_debug libuv_readfile_debug.c)
target_link_libraries(libuv_readfile_debug uv)

add_executable(libuv_http_connect libuv_http_connect.cpp)
target_link_libraries(libuv_http_connect uv)