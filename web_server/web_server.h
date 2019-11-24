#pragma once

#include <thread>
#include <string>

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
    void register_module(const std::string& name = "/", const std::string& username = "", const std::string& password = "") {    
        if (T::init()) {
            module_context newModule;
            newModule.name = name;
            newModule.callback = T::handle;
            newModule.authentication = {username, password};
            
            m_context.moduleMap[name] = newModule;
            log_status("Loaded module '" + std::string(name) + "'");
        } else {
            log_warning("Could not load module '" + std::string(name) + "'");
        }
    }

    bool init(config newConfig = config());
    bool run();
private:
    void log_status(const std::string& msg);
    void log_warning(const std::string& msg);
    void log_error(const std::string& msg);
    
    network::tcp_socket<network::ipv4_addr> m_socket;
    network::ssl_context<network::ipv4_addr> m_sslContext;
    shared_context m_context;    
};
}



