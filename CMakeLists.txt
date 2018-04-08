cmake_minimum_required(VERSION 2.6)
project(web_server)

add_executable(web_server main.cpp)

install(TARGETS web_server RUNTIME DESTINATION bin)
