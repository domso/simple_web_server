#pragma once

#include <functional>
#include <vector>
#include <memory>
#include <mutex>

#include "network/pkt_buffer.h"

namespace web_server {    
    struct unique_context {
        network::pkt_buffer recv_buffer = {1024};
        std::string response_header;
        std::vector<char> response_data;
        std::vector<char> recv_data;
        bool is_native = false;
        uint64_t id;
        bool valid = true;
        
        std::mutex native_mutex;
        std::vector<std::vector<char>> native_send_data;
    }; 
}

