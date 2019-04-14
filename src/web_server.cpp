#include "web_server.h"

bool web_server::web_server::init(config newConfig) {
    m_context.currentConfig = newConfig;
    
    m_socket.accept_on(newConfig.port, newConfig.max_pending);
    if (!m_sslContext.init(newConfig.cert, newConfig.key)) {
        return false;
    }
    
    return true;
}


bool web_server::web_server::run() {
    while (true) {
        auto tcpConnection = m_socket.accept_connection();
        if (!tcpConnection) {
            // skt is broken
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
