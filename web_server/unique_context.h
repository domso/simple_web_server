#pragma once

#include <functional>
#include <vector>
#include <memory>
#include <mutex>

#include "network/pkt_buffer.h"
#include "native_interface.h"

namespace web_server {    
    struct unique_context {
        network::pkt_buffer recv_buffer = {1024};
        std::string response_header;
        std::vector<char> response_data;
        std::vector<char> recv_data;
        bool is_native = false;
        uint64_t id;
        bool valid = true;
        
        native_interface native_caller;
    }; 
}

