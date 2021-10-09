#pragma once

#include <functional>
#include <vector>

namespace web_server {
    class on_destroy_handle {
    public:  
        ~on_destroy_handle() {
            for (const auto& call : m_calls) {
                call();
            }
        }
        
        on_destroy_handle(on_destroy_handle& copy) = delete;
        on_destroy_handle(on_destroy_handle&& move) : m_calls(std::move(move.m_calls)) {}
        void operator=(on_destroy_handle& copy) = delete;
        void operator=(on_destroy_handle&& move) {
            m_calls = std::move(move.m_calls);
        }   
        
        void on_destroy(std::function<void()> call) {
            m_calls.push_back(call);
        }
        
    private:
        std::vector<std::function<void()>> m_calls;
    };
}

