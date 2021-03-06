#pragma once

#include <memory>

#include "network/ssl_connection.h"
#include "shared_context.h"

namespace web_server {

class connection_thread {
public:
    static void main(network::ssl_connection<network::ipv4_addr> connection, shared_context& context);
private:
    static std::string build_response(const std::unordered_map<std::string, std::string>& fieldMap);
};

}
