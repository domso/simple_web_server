#pragma once

#include <thread>
#include <string>
#include <optional>

#include "network/ssl_connection.h"
#include "network/wait_ops.h"

#include "web_server/http/requester.h"
#include "web_server/unique_context.h"
#include "web_server/config.h"

namespace web_server::http {
    class executor {
    public: 
        executor(config& current_config);
        
        template <typename T>
        void register_module(T& mod, const std::string& name = "/", const std::string& username = "", const std::string& password = "") {    
            m_requester.register_module<T>(mod, name, username, password);
        }
        
        network::wait_ops execute(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context) const ;
    private:    
        std::optional<network::wait_ops> recv_data(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context) const;
        std::optional<network::wait_ops> send_header(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context) const;
        std::optional<network::wait_ops> send_data(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context) const;
        std::string build_response(const std::unordered_map<std::string, std::string>& field_map) const;
        
        requester m_requester;
    };
}




