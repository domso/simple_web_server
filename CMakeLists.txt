cmake_minimum_required(VERSION 2.6)
project(web_server)

include_directories(./)
include_directories(/usr/include/openssl)
include_directories(${OPENSSL_INCLUDE_DIR})
find_package(OpenSSL REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FALGS} -g -std=c++20 -pthread -fconcepts")

add_library(web_server SHARED
    util/base64.cpp
    util/logger.cpp
    util/signal_interrupt.cpp
    web_server/http/status_code.cpp 
    web_server/http/parser.cpp 
    web_server/http/requester.cpp 
    web_server/http/executor.cpp    
    
    web_server/ssl_accept/executor.cpp
    
    web_server/native/send_recv_interface.cpp
    web_server/native/handle.cpp
    web_server/native/executor.cpp
    
    web_server/web_server.cpp
    modules/file_loader.cpp
    modules/web_socket/accept.cpp
    modules/web_socket/frame_decoder.cpp
    modules/web_socket/frame_encoder.cpp
    modules/web_socket/web_socket.cpp    
)

target_link_libraries(web_server ${OPENSSL_LIBRARIES})

add_executable(test examples/main.cpp)
target_link_libraries(test web_server)
