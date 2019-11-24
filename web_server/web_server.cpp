#include "web_server.h"

#include "signal.h"

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
    while (true) {
        auto tcpConnection = m_socket.accept_connection();
        if (!tcpConnection) {
            log_error("Socket is broken");
            return false;
        }
        
        auto sslConnection = m_sslContext.accept(std::move(*tcpConnection));
        if (sslConnection) {            
            std::thread newThread(&connection_thread::main, std::move(*sslConnection), std::ref(m_context));
            newThread.detach();
        }
    }
    
    return true;
}


void web_server::web_server::log_status(const std::string& msg) {
    if (m_context.currentConfig.enable_log_status) {
        std::cout << m_context.currentConfig.log_status + " " + msg << std::endl;
    }
}

void web_server::web_server::log_warning(const std::string& msg) {
    if (m_context.currentConfig.enable_log_warning) {
        std::cout << m_context.currentConfig.log_warning + " " + msg << std::endl;
    }
}

void web_server::web_server::log_error(const std::string& msg) {
    if (m_context.currentConfig.enable_log_error) {
        std::cout << m_context.currentConfig.log_error + " " + msg << std::endl;
    }
}
