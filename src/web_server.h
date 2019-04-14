#pragma once

#include <thread>
#include "network/tcp_socket.h"
#include "network/tcp_connection.h"
#include "network/ssl_context.h"
#include "network/ssl_connection.h"
#include "connection_thread.h"
#include "shared_context.h"
#include "config.h"

namespace web_server {

class web_server {
public:
    template <typename T>
    void register_new_handle() {
        m_context.callbackMap[T::name] = (std::pair<std::vector<char>, int>(*)(void*)) T::handle;
    }

    bool init(config newConfig = config());
    bool run();
private:
    network::tcp_socket<network::ipv4_addr> m_socket;
    network::ssl_context<network::ipv4_addr> m_sslContext;
    shared_context m_context;    
};
}

using namespace web_server;


