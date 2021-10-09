#include "web_server.h"

#include <iostream>

#include "signal.h"

#include "worker.h"

#include "network/status.h"
#include "http_parser.h"
#include "http_request.h"
#include "web_socket/frame_encoder.h"


bool web_server::web_server::init(config newConfig) {
    m_context.currentConfig = newConfig;
    
    log_status("Initiate server");
    m_socket.accept_on(newConfig.port, newConfig.max_pending);
    if (!m_sslContext.init(newConfig.cert, newConfig.key)) {
        log_error("Could not load server");
        return false;
    }
    
    return true;
}

bool web_server::web_server::run() {
    log_status("Start server");
    signal(SIGPIPE, SIG_IGN);
    
    std::vector<std::unique_ptr<worker<network::ssl_connection<network::ipv4_addr>, unique_context>>> http_workers;
    
    //FIXME configure number of threads and timeouts
    for (size_t i = 0; i < m_context.currentConfig.num_worker; i++) {
        http_workers.push_back(std::make_unique<worker<network::ssl_connection<network::ipv4_addr>, unique_context>>([&](network::ssl_connection<network::ipv4_addr>& conn, unique_context& context){   
            if (context.is_websocket) {
                return websocket_handler(conn, context);
            } else {
                return http_handler(conn, context);                        
            }
        }, 1000, 10));
    }
    
    
    
    //FIXME add load balancer
    //should not only give connections to worker 0
    worker<network::ssl_connection<network::ipv4_addr>, int> accept_worker([&](network::ssl_connection<network::ipv4_addr>& conn, int&){   
       return accept_handler(conn, *http_workers[0]);        
    }, 1000, 10);   
    
    while (true) {
        auto tcpConnection = m_socket.accept_connection();
        if (!tcpConnection) {
            log_error("Socket is broken");
            return false;
        }
        tcpConnection->set_blocking(false);
        
        log_status("New tcp connection accepted");
        auto sslConnection = m_sslContext.convert(std::move(*tcpConnection));
        accept_worker.add_socket(std::move(sslConnection));
    }
    
    return true;
}

network::wait_ops web_server::web_server::accept_handler(network::ssl_connection<network::ipv4_addr>& conn, worker<network::ssl_connection<network::ipv4_addr>, web_server::web_server::unique_context>& connection_worker) {
    auto [status, code] = conn.accept();
    
    switch (status) {
        case network::status::ok: {
            log_status("SSL accepted");
            connection_worker.add_socket(std::move(conn));
            return network::wait_ops::remove;
        }
        case network::status::retry_read: return network::wait_ops::wait_read;
        case network::status::retry_write: return network::wait_ops::wait_write;
        case network::status::error: return network::wait_ops::remove;
        default: return network::wait_ops::remove;
    }            
}

network::wait_ops web_server::web_server::http_handler(network::ssl_connection<network::ipv4_addr>& conn, web_server::web_server::unique_context& context) {    
    if (auto result = http_handler_recv(conn, context)) {
        return *result;
    }
    if (auto result = http_handler_send_header(conn, context)) {
        return *result;
    }
    if (auto result = http_handler_send_data(conn, context)) {
        return *result;
    }
    
    return network::wait_ops::wait_read_write;
}

std::optional<network::wait_ops> web_server::web_server::http_handler_recv(network::ssl_connection<network::ipv4_addr>& conn, web_server::web_server::unique_context& context) {
    if (context.response_header.empty() && context.response_data.empty()) {        
        auto [status, code] = conn.recv_pkt(context.recv_buffer);        
        switch (status) {
            case network::status::ok: { 
                auto read_region = context.recv_buffer.readable_region();
                
                for (size_t i = 3; i < read_region.size(); i++) {
                    if (read_region.data()[i - 0] == '\n' &&
                        read_region.data()[i - 1] == '\r' &&
                        read_region.data()[i - 2] == '\n' &&
                        read_region.data()[i - 3] == '\r'                
                    ) { 
                        std::string header(reinterpret_cast<char*>(read_region.data()), i + 1);
                        auto request = http_parser::parse_request(header);
                        auto response = http_request::handle_request(request, m_context);
                        
                        context.response_header = build_response(response.first);
                        context.response_data = std::move(response.second);                        
                        context.recv_buffer.read(read_region.splice(0, i + 1));
                        
                        if (response.first["STATUS"] == "101 Switching Protocols") {
                            context.is_websocket = true;
                        }
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
            case network::status::error: return network::wait_ops::remove;
            default: return network::wait_ops::remove;
        }
    }
    
    return std::nullopt;
}

std::optional<network::wait_ops> web_server::web_server::http_handler_send_header(network::ssl_connection<network::ipv4_addr>& conn, web_server::web_server::unique_context& context) {
    if (!context.response_header.empty()) {        
        network::memory_region region;
        region.use(context.response_header);
        
        auto [status, code] = conn.send_data(region);
        
        switch (status) {
            case network::status::ok: {
                context.response_header.clear();
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

std::optional<network::wait_ops> web_server::web_server::http_handler_send_data(network::ssl_connection<network::ipv4_addr>& conn, web_server::web_server::unique_context& context) {
    if (!context.response_data.empty()) {       
        network::memory_region region;
        region.use(context.response_data);
        
        auto [status, code] = conn.send_data(region);
        
        switch (status) {
            case network::status::ok: {
                context.response_data.clear();
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

network::wait_ops web_server::web_server::websocket_handler(network::ssl_connection<network::ipv4_addr>& conn, web_server::web_server::unique_context& context) {
    if (auto result = websocket_recv(conn, context)) {
        return *result;
    }    
        
    if (auto result = http_handler_send_data(conn, context)) {
        return *result;
    }
    
    return network::wait_ops::wait_read_write;
}

std::optional<network::wait_ops> web_server::web_server::websocket_recv(network::ssl_connection<network::ipv4_addr>& conn, web_server::web_server::unique_context& context) {   
    if (context.response_data.empty()) {
        auto [status, code] = conn.recv_pkt(context.recv_buffer);        
        switch (status) {
            case network::status::ok: { 
                auto read_region = context.recv_buffer.readable_region();
                                
                auto n = context.frame_decoder.unpack_data(read_region, [&](const network::memory_region region, const uint8_t header) {                    
                    region.push_back_into(context.recv_data);
                    
                    if ((header & 128) > 0) {                                         
                        std::string response = "hello world";
                        network::memory_region response_region;
                        response_region.use(response);
                        context.frame_encoder.pack_data(response_region, [&](const network::memory_region region) {
                            region.push_back_into(context.response_data);
                        });
                                                
                        context.recv_data.clear();
                    }                    
                });
                
                context.recv_buffer.read(read_region.splice(0, n));

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


std::string web_server::web_server::build_response(const std::unordered_map<std::string, std::string>& fieldMap) const {        
    std::string result;    
    auto it = fieldMap.find("STATUS");
    
    if (it != fieldMap.end()) {
         result = "HTTP/1.1 " + it->second + "\r\n";   
    }    
    
    for (auto& keyValue : fieldMap) {
        if (keyValue.first != "STATUS") {
            result += keyValue.first + ": " + keyValue.second + "\r\n";
        }
    }
    
    result += "\r\n";
    
    return result;
}

void web_server::web_server::log_status(const std::string& msg) const {
    if (m_context.currentConfig.enable_log_status) {
        std::cout << m_context.currentConfig.log_status + " " + msg << std::endl;
    }
}

void web_server::web_server::log_warning(const std::string& msg) const {
    if (m_context.currentConfig.enable_log_warning) {
        std::cout << m_context.currentConfig.log_warning + " " + msg << std::endl;
    }
}

void web_server::web_server::log_error(const std::string& msg) const {
    if (m_context.currentConfig.enable_log_error) {
        std::cout << m_context.currentConfig.log_error + " " + msg << std::endl;
    }
}
