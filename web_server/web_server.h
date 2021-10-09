#pragma once

#include <thread>
#include <string>
#include <optional>

#include "network/tcp_socket.h"
#include "network/tcp_connection.h"
#include "network/ssl_context.h"
#include "network/ssl_connection.h"
#include "network/pkt_buffer.h"
#include "shared_context.h"
#include "config.h"
#include "worker.h"
#include "network/wait_ops.h"
#include "web_socket/frame_encoder.h"
#include "web_socket/frame_decoder.h"

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
    struct unique_context {
        network::pkt_buffer recv_buffer = {1024};
        std::string response_header;
        std::vector<char> response_data;
        std::vector<char> recv_data;
        web_socket::frame_encoder frame_encoder = 1024;
        web_socket::frame_decoder frame_decoder;
        bool is_websocket = false;
    };
    
    network::wait_ops accept_handler(network::ssl_connection<network::ipv4_addr>& conn, worker<network::ssl_connection<network::ipv4_addr>, unique_context>& connection_worker);    
    
    network::wait_ops http_handler(network::ssl_connection<network::ipv4_addr>& conn, unique_context& context);
    
    std::optional<network::wait_ops> http_handler_recv(network::ssl_connection<network::ipv4_addr>& conn, unique_context& context);
    std::optional<network::wait_ops> http_handler_send_header(network::ssl_connection<network::ipv4_addr>& conn, unique_context& context);
    std::optional<network::wait_ops> http_handler_send_data(network::ssl_connection<network::ipv4_addr>& conn, unique_context& context);
    
    network::wait_ops websocket_handler(network::ssl_connection<network::ipv4_addr>& conn, unique_context& context);
    std::optional<network::wait_ops> websocket_recv(network::ssl_connection<network::ipv4_addr>& conn, unique_context& context);
    
    std::string build_response(const std::unordered_map<std::string, std::string>& fieldMap) const;
    
    void log_status(const std::string& msg) const;
    void log_warning(const std::string& msg) const;
    void log_error(const std::string& msg) const;   
    
    network::tcp_socket<network::ipv4_addr> m_socket;
    network::ssl_context<network::ipv4_addr> m_sslContext;
    shared_context m_context;
    size_t current_selected_worker = 0;
};
}



