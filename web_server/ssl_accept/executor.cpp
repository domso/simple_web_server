#include "executor.h"

#include "util/logger.h"

network::wait_ops web_server::ssl_accept::executor::execute(
    network::ssl_connection<network::ipv4_addr>& conn,
    worker<network::ssl_connection<network::ipv4_addr>,
    web_server::unique_context>& connection_worker
) {
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
