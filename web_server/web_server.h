#pragma once

#include <string>
#include <optional>

#include "network/tcp_socket.h"
#include "network/ssl_context.h"
#include "network/ssl_connection.h"
#include "config.h"
#include "worker.h"

#include "unique_context.h"
#include "util/logger.h"
#include "ssl_accept/executor.h"
#include "http/executor.h"
#include "native/executor.h"

namespace web_server {

class web_server {
public:
    web_server();
    
    template <typename T>
    void register_module(T& mod, const std::string& name = "/", const std::string& username = "", const std::string& password = "") {    
        m_http_executor.register_module<T>(mod, name, username, password);
        util::logger::log_status("Loaded module '" + std::string(name) + "'");
    }

    bool init(config new_config = config());
    bool run();
private:    
    void start_http_worker();
    void start_accept_worker();
    
    ssl_accept::executor m_accept_executor;
    http::executor m_http_executor;
    native::executor m_native_executor;
    
    std::vector<std::unique_ptr<worker<network::ssl_connection<network::ipv4_addr>, unique_context>>> m_http_workers;
    std::unique_ptr<worker<network::ssl_connection<network::ipv4_addr>, void>> m_accept_worker;
    size_t m_select_worker;
    
    network::tcp_socket<network::ipv4_addr> m_socket;
    network::ssl_context<network::ipv4_addr> m_ssl_context;
    config m_config;
};
}



