#include "executor.h"

#include "network/status.h"

#include "util/logger.h"

network::wait_ops web_server::native::executor::execute(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context) const {
    if (!context->is_valid.load(std::memory_order_relaxed)) {
        util::logger::log_debug("Close connection to " + context->name);
        return network::wait_ops::remove;
    }
    
    if (context->response_data.empty()) {
        context->native_if.push_into(context->response_data);
    }    
    
    if (auto result = native_recv(conn, context)) {
        return *result;
    }    
        
    if (auto result = native_send(conn, context)) {
        return *result;
    }
    
    util::logger::log_warning("Nothing to do for " + context->name);
    
    return network::wait_ops::wait_read_write;
}

std::optional<network::wait_ops> web_server::native::executor::native_recv(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context) const {   
    if (context->response_data.empty()) {
        auto [status, code] = conn.recv_pkt(context->recv_buffer);        
        switch (status) {
            case network::status::ok: { 
                auto read_region = context->recv_buffer.readable_region();
                util::logger::log_debug("Received " + std::to_string(read_region.size()) + " bytes from " + context->name);
                
                if (context->native_if.on_recv) {
                    auto n = context->native_if.on_recv(read_region);
                    context->recv_buffer.read(read_region.splice(0, n));
                } else {
                    util::logger::log_warning("No receive callback defined for incoming traffic. Discarding data.");
                    context->recv_buffer.read(read_region);
                }                

                break;
            }
            case network::status::retry_read: {
                return network::wait_ops::wait_read;                    
            }
            case network::status::retry_write: {
                return network::wait_ops::wait_write;                    
            }
            case network::status::error: return network::wait_ops::remove;
            default: return network::wait_ops::remove;
        }
    }
    
    return std::nullopt;
}

std::optional<network::wait_ops> web_server::native::executor::native_send(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context) const {
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
                return network::wait_ops::remove;
            }
        }              
    }
    
    return std::nullopt;
}
