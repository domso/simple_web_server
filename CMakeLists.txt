cmake_minimum_required(VERSION 2.6)
project(web_server)

include_directories(../network/)
include_directories(/usr/include/openssl)
include_directories(${OPENSSL_INCLUDE_DIR})
find_package(OpenSSL REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FALGS} -O3 -std=c++17 -pthread")

add_library(web_server src/connection_thread.cpp src/http_parser.cpp src/http_request.cpp src/file_loader.cpp src/http_status_code.cpp src/web_server.cpp)
target_link_libraries(web_server ${OPENSSL_LIBRARIES})

add_executable(test tests/main.cpp)
target_link_libraries(test web_server)


