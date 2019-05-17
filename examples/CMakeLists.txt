include_directories(../deps/http-parser)

add_executable(http_parser_example http_parser_example.cpp ../deps/http-parser/http_parser.c)