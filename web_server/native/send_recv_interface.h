#pragma once

#include <mutex>
#include <functional>
#include <vector>

#include "network/memory_region.h"

namespace web_server::native {    
    class send_recv_interface {
    public:
        ~send_recv_interface();
        std::function<size_t(const network::memory_region)> on_recv;
        std::function<void()> on_close;        
        
        void send(std::vector<char>&& vec);   
        void send(network::memory_region data);         
        
        void push_into(std::vector<char>& data);
        void close();
    private:
        std::mutex m_mutex;
        std::vector<std::vector<char>> m_data;
    };
}
