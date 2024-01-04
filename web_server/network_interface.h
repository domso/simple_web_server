#pragma once

#include <thread>
#include <string>
#include <optional>

#include "util/logger.h"

#include "network/ssl_connection.h"
#include "network/wait_ops.h"
#include "network/status.h"

#include "web_server/module/adapter.h"
#include "web_server/unique_context.h"

namespace web_server {
    template<typename T_config, typename T, typename... Ts>
    class network_interface {
    public: 
        template<typename Treq>
        auto& get_module() {
            return m_mod_adapter.template get_module<Treq>();
        }
        template<typename Treq>
        const auto& get_module() const {
            return m_mod_adapter.template get_module<Treq>();
        }
        
        network::wait_ops ssl_accept(
            network::ssl_connection<network::ipv4_addr>& conn,
            worker<network::ssl_connection<network::ipv4_addr>,
            unique_context<T, Ts...>>& connection_worker
        ){
            auto [status, code] = conn.accept();
            
            switch (status) {
                case network::status::ok: {
                    util::logger::log_debug("New SSL connection from " + conn.to_string());
                    connection_worker.add_socket(std::move(conn));
                    return network::wait_ops::remove;
                }
                case network::status::retry_read: return network::wait_ops::wait_read;
                case network::status::retry_write: return network::wait_ops::wait_write;
                case network::status::error: return network::wait_ops::remove;
                default: return network::wait_ops::remove;
            }            
        }
        network::wait_ops update(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context<T, Ts...>>& context) {
            if (context->is_native) {
                m_mod_adapter.handle_native_update(context->native_module_id, context->response_data, context);
            }
    
            if (auto result = recv_data(conn, context)) {
                return *result;
            }
            if (auto result = send_data(conn, context)) {
                return *result;
            }
            
            util::logger::log_warning("Nothing to do for " + context->name);

            if (context->is_native) {
                if (!m_mod_adapter.handle_native_status(context->native_module_id, context)) {
                    util::logger::log_debug("Close connection to " + context->name);
                    return network::wait_ops::remove;
                }
            }
            
            return network::wait_ops::wait_read_write;
        }
    private:    
        std::optional<network::wait_ops> recv_data(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context<T, Ts...>>& context) {
            if (context->response_data.empty()) {        
                auto [status, code] = conn.recv_pkt(context->recv_buffer);
                switch (status) {
                    case network::status::ok: { 
                        auto read_region = context->recv_buffer.readable_region();
                        util::logger::log_debug("Received " + std::to_string(read_region.size()) + " bytes from " + context->name);

                        if (!context->is_native) {
                            auto n = m_mod_adapter.handle_http_request(read_region, context);
                            context->recv_buffer.read(read_region.splice(0, n));
                        } else {
                            auto n = m_mod_adapter.handle_native_request(context->native_module_id, read_region, context);
                            context->recv_buffer.read(read_region.splice(0, n));
                        }

                        if (context->is_native) {
                            if (!m_mod_adapter.handle_native_status(context->native_module_id, context)) {
                                util::logger::log_debug("Close connection to " + context->name);
                                return network::wait_ops::remove;
                            }
                        }
                        break;
                    }
                    case network::status::retry_read: {
                        return network::wait_ops::wait_read;                    
                    }
                    case network::status::retry_write: {
                        return network::wait_ops::wait_write;                    
                    }
                    case network::status::error: 
                    default: {
                        util::logger::log_debug("Close connection to " + context->name + " due to error");
                        return network::wait_ops::remove;
                    }
                }
            }
            
            return std::nullopt;
        }
        std::optional<network::wait_ops> send_data(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context<T, Ts...>>& context) {
            if (!context->response_data.empty()) {   
                network::memory_region region;
                region.use(context->response_data);
                
                util::logger::log_debug("Send " + std::to_string(region.size()) + " bytes to " + context->name);
                auto [status, code] = conn.send_data(region);
                
                switch (status) {
                    case network::status::ok: {
                        context->response_data.clear();
                        break;
                    }
                    case network::status::retry_read: {
                        return network::wait_ops::wait_read;
                    }
                    case network::status::retry_write: {
                        return network::wait_ops::wait_write;
                    }
                    case network::status::error: {
                        util::logger::log_debug("Close connection to " + context->name + " due to error");
                        return network::wait_ops::remove;
                    }
                }              
            }
            
            return std::nullopt;
        }
        
        module::adapter<T_config, T, Ts...> m_mod_adapter;
    };
}
