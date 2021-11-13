#pragma once

#include <mutex>
#include <functional>
#include <vector>

#include "network/memory_region.h"

#include "unique_context.h"

namespace web_server {
    class native_handler {
    public:
        std::function<bool(network::memory_region)> on_recv;
        std::function<void()> on_error;
        
        void send(std::vector<char>&& vec) {
            std::unique_lock<std::mutex> ul(m_mutex);
            m_data.push_back(std::move(vec));
        }
        
        void push_into(std::vector<char> data) {
            std::unique_lock<std::mutex> ul(m_mutex);
            for (auto& v : m_data) {
                data.insert(data.end(), v.begin(), v.end());
            }
            
            m_data.clear();
        }        
    private:
        std::mutex m_mutex;
        std::vector<std::vector<char>> m_data;
    };
}
