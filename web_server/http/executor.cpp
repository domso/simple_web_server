#include "executor.h"

#include "network/status.h"

#include "web_server/http/requester.h"
#include "util/logger.h"

web_server::http::executor::executor(web_server::config& current_config) : m_requester(current_config) {}

network::wait_ops web_server::http::executor::execute(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context) const {    
    if (auto result = recv_data(conn, context)) {
        return *result;
    }
    if (auto result = send_header(conn, context)) {
        return *result;
    }
    if (auto result = send_data(conn, context)) {
        return *result;
    }
    
    util::logger::log_warning("Nothing to do for " + context->name);
    
    return network::wait_ops::wait_read_write;
}

std::optional<network::wait_ops> web_server::http::executor::recv_data(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context) const {
    if (context->response_header.empty() && context->response_data.empty()) {        
        auto [status, code] = conn.recv_pkt(context->recv_buffer);
        switch (status) {
            case network::status::ok: { 
                auto read_region = context->recv_buffer.readable_region();
                util::logger::log_debug("Received " + std::to_string(read_region.size()) + " bytes from " + context->name);
                
                for (size_t i = 3; i < read_region.size(); i++) {
                    if (read_region.data()[i - 0] == '\n' &&
                        read_region.data()[i - 1] == '\r' &&
                        read_region.data()[i - 2] == '\n' &&
                        read_region.data()[i - 3] == '\r'                
                    ) {                         
                        std::string header = read_region.splice(0, i + 1).export_to<std::string>();
                        
                        auto response = m_requester.handle_request(header, context);
                        
                        context->response_header = build_response(response.first);
                        context->response_data = std::move(response.second);                        
                        context->recv_buffer.read(read_region.splice(0, i + 1));
                    }
                }
                // FIXME!
                context->recv_buffer.read(read_region);
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

std::optional<network::wait_ops> web_server::http::executor::send_header(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context) const {
    if (!context->response_header.empty()) {        
        network::memory_region region;
        region.use(context->response_header);
        
        util::logger::log_debug("Send " + std::to_string(region.size()) + " bytes to " + context->name);
        auto [status, code] = conn.send_data(region);
        
        switch (status) {
            case network::status::ok: {
                context->response_header.clear();
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

std::optional<network::wait_ops> web_server::http::executor::send_data(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context) const {
    if (!context->response_data.empty()) {   
        network::memory_region region;
        region.use(context->response_data);
        
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

std::string web_server::http::executor::build_response(const std::unordered_map<std::string, std::string>& field_map) const {        
    std::string result;    
    auto it = field_map.find("STATUS");
    
    if (it != field_map.end()) {
         result = "HTTP/1.1 " + it->second + "\r\n";   
    }    
    
    for (auto& key_value : field_map) {
        if (key_value.first != "STATUS") {
            result += key_value.first + ": " + key_value.second + "\r\n";
        }
    }
    
    result += "\r\n";
    
    return result;
}
