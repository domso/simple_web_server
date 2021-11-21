#pragma once

#include <functional>
#include <vector>
#include <memory>
#include <mutex>
#include <iostream>
#include <atomic>

#include "network/pkt_buffer.h"
#include "native/send_recv_interface.h"
#include "util/logger.h"

namespace web_server {    
    struct unique_context {        
        ~unique_context() {
            util::logger::log_debug("Delete context of " + name);            
        }
        network::pkt_buffer recv_buffer = {1024};
        std::string response_header;
        std::vector<char> response_data;
        std::vector<char> recv_data;
        bool is_native = false;
        std::atomic<bool> is_valid = true;
        uint64_t id;
        std::function<void()> force_call;
        std::string name;
        
        native::send_recv_interface native_if;
        
        void close() {           
            native_if.close();
            is_valid = false;
        }
    }; 
}

