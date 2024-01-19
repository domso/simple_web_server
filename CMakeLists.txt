cmake_minimum_required(VERSION 2.6)
project(web_server)

include_directories(./)
include_directories(/usr/include/openssl)
include_directories(${OPENSSL_INCLUDE_DIR})
find_package(OpenSSL REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FALGS} -g -std=c++20 -pthread -fconcepts -O3")

add_library(web_server SHARED
    util/base64.cpp
    util/logger.cpp
    util/signal_interrupt.cpp
    web_server/http/status_code.cpp 
    web_server/http/parser.cpp 
    
    modules/file_loader.cpp
    modules/web_socket/frame_decoder.cpp
    modules/web_socket/frame_encoder.cpp
    modules/web_socket/web_socket.cpp    
)

add_executable(proxy_sender 
    util/proxy_sender.cpp
)

add_executable(proxy_receiver
    util/proxy_receiver.cpp
)

target_link_libraries(web_server ${OPENSSL_LIBRARIES})

add_executable(test examples/main.cpp)
target_link_libraries(test web_server)
