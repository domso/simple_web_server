#pragma once

#include <string>
#include <optional>

#include "network/ssl_connection.h"
#include "network/wait_ops.h"
#include "network/status.h"

#include "web_server/worker.h"
#include "web_server/unique_context.h"

namespace web_server::ssl_accept {
    class executor {
    public:  
        network::wait_ops execute(
            network::ssl_connection<network::ipv4_addr>& conn,
            worker<network::ssl_connection<network::ipv4_addr>,
            unique_context>& connection_worker
        );
    };
}




