#pragma once

#include <string>
#include <optional>

#include "network/ssl_connection.h"
#include "network/wait_ops.h"

#include "web_server/unique_context.h"

namespace web_server::native {
    class executor {
    public:  
        network::wait_ops execute(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context) const;
    private:
        std::optional<network::wait_ops> native_recv(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context) const;
        std::optional<network::wait_ops> native_send(network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context>& context) const;
    };
}




