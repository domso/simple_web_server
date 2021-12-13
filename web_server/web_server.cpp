#include "web_server.h"

#include <memory>

#include "signal.h"

#include "network/status.h"
#include "util/signal_interrupt.h"

#include "http/parser.h"
#include "http/requester.h"
#include "worker.h"

web_server::web_server::web_server() : m_http_executor(m_config) {}

bool web_server::web_server::init(config new_config) {
    m_config = new_config;
    
    util::logger::log_status("Initiate server");
    m_socket.accept_on(new_config.port, new_config.max_pending);
    if (!m_ssl_context.init(new_config.cert, new_config.key)) {
        util::logger::log_error("Could not load server");
        return false;
    }
    
    return true;
}

bool web_server::web_server::run() {
    util::logger::log_status("Start server");
    signal(SIGPIPE, SIG_IGN);
    util::signal_interrupt itr([this](){
       m_socket.close_socket(); 
    });
        
    start_http_worker();
    start_accept_worker();
    
    while (auto tcp_connection = m_socket.accept_connection()) {
        util::logger::log_status("New TCP connection from " + tcp_connection->get_addr().ip() + ":" + std::to_string(tcp_connection->get_addr().port()));
        tcp_connection->set_blocking(false);
        auto ssl_connection = m_ssl_context.convert(std::move(*tcp_connection));
        m_accept_worker->add_socket(std::move(ssl_connection));                           
    }
        
    util::logger::log_status("Stop server");
    return true;
}

void web_server::web_server::start_http_worker() {
    typedef worker<network::ssl_connection<network::ipv4_addr>, ::web_server::unique_context> worker_type;
    
    for (size_t i = 0; i < m_config.num_worker; i++) {
        auto call = [&](network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context){
            if (context->is_native) {
                return m_native_executor.execute(conn, context);
            } else {
                return m_http_executor.execute(conn, context);                        
            }
        };                
        m_http_workers.push_back(std::unique_ptr<worker_type>(new worker_type(call, m_config.worker_poll_timeout, m_config.worker_poll_buffer_size)));
    }
}

void web_server::web_server::start_accept_worker() {
    typedef worker<network::ssl_connection<network::ipv4_addr>, void> worker_type;
    
    m_select_worker = 0;
    auto call = [&](network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<void>&){   
        auto id = m_select_worker++;
        if (m_select_worker == m_http_workers.size()) {
            m_select_worker = 0;
        }
        return m_accept_executor.execute(conn, *m_http_workers[id]);        
    };
    
    m_accept_worker = std::unique_ptr<worker_type>(new worker_type(call, m_config.accept_poll_timeout, m_config.accept_poll_buffer_size)); 
}

