include_directories(../deps/http-parser)

add_executable(http_parser_example http_parser_example.cpp ../deps/http-parser/http_parser.c)
add_executable(libuv_debug libuv_debug.cpp)
target_link_libraries(libuv_debug uv)