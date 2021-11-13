#pragma once

#include <thread>
#include <string>
#include <optional>

#include "network/tcp_socket.h"
#include "network/tcp_connection.h"
#include "network/ssl_context.h"
#include "network/ssl_connection.h"
#include "network/pkt_buffer.h"
#include "config.h"
#include "worker.h"
#include "network/wait_ops.h"

#include "http_request.h"
#include "unique_context.h"

namespace web_server {

class web_server {
public:
    web_server();
    
    template <typename T>
    void register_module(T& mod, const std::string& name = "/", const std::string& username = "", const std::string& password = "") {    
        m_requester.register_module<T>(mod, name, username, password);
        log_status("Loaded module '" + std::string(name) + "'");
    }

    bool init(config newConfig = config());
    bool run();
private:    
    network::wait_ops accept_handler(network::ssl_connection<network::ipv4_addr>& conn, worker<network::ssl_connection<network::ipv4_addr>, unique_context>& connection_worker);    
    
    network::wait_ops http_handler(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context);
    
    std::optional<network::wait_ops> http_handler_recv(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context);
    std::optional<network::wait_ops> http_handler_send_header(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context);
    std::optional<network::wait_ops> http_handler_send_data(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context);
    
    network::wait_ops native_handler(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context);
    std::optional<network::wait_ops> native_recv(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context);
    
    std::string build_response(const std::unordered_map<std::string, std::string>& fieldMap) const;
    
    void log_status(const std::string& msg) const;
    void log_warning(const std::string& msg) const;
    void log_error(const std::string& msg) const;   
    
    std::vector<std::unique_ptr<worker<network::ssl_connection<network::ipv4_addr>, unique_context>>> m_http_workers;
    
    network::tcp_socket<network::ipv4_addr> m_socket;
    network::ssl_context<network::ipv4_addr> m_sslContext;
    config m_config;
    http_request m_requester;
    size_t current_selected_worker = 0;
};
}



