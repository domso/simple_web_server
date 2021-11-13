cmake_minimum_required(VERSION 2.6)
project(web_server)

include_directories(../network/)
include_directories(/usr/include/openssl)
include_directories(${OPENSSL_INCLUDE_DIR})
find_package(OpenSSL REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FALGS} -O3 -std=c++17 -pthread")

add_library(web_server SHARED
    web_server/http_parser.cpp 
    web_server/http_request.cpp 
    web_server/http_status_code.cpp 
    web_server/web_server.cpp
    web_server/native_interface.cpp
    web_server/native_handle.cpp
    modules/file_loader.cpp
)

target_link_libraries(web_server ${OPENSSL_LIBRARIES})

add_executable(test examples/main.cpp)
target_link_libraries(test web_server)

