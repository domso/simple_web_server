#pragma once

#include <mutex>
#include <functional>
#include <vector>

#include "network/memory_region.h"

namespace web_server {    
    class native_interface {
    public:
        ~native_interface();
        std::function<bool(const network::memory_region)> on_recv;
        std::function<void()> on_close;        
        
        void send(std::vector<char>&& vec);        
        void push_into(std::vector<char> data);
    private:
        std::mutex m_mutex;
        std::vector<std::vector<char>> m_data;
    };
}
