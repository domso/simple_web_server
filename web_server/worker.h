#pragma once

#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>

#include "network/socket_container.h"

namespace web_server {
    template<typename skt_T, typename arg_T>
    class worker {
    public:
        worker(std::function<network::wait_ops(skt_T&, arg_T&)> call, const size_t poll_timeout, const size_t poll_buffer_size) : m_sockets(poll_buffer_size) {            
            m_running = true;
            m_thread = std::thread([call, poll_timeout, this]() {                            
                while(m_running.load(std::memory_order_relaxed)) {
                    clear_backlog();
                    
                    if (on_interrupt) {
                        on_interrupt();
                    }
                    
                    m_sockets.wait([&](item& c) {
                        return call(c.skt, c.arg);
                    }, poll_timeout);
                }                
            });            
        }
        
        worker(const int s) : m_sockets(s){}
        
        ~worker() {
            m_running = false;           
            m_sockets.interrupt(); 
            if (m_thread.joinable()) {
                m_thread.join();
            }
        }
        
/*        worker(const worker&) = delete;
        worker(const worker&&) = delete;
        void operator=(const worker&) = delete;
        void operator=(const worker&&) = delete;       */    
        
        void add_socket(skt_T&& skt) {
            {
                std::lock_guard<std::mutex> lg(m_mutex);
                m_backlog.push_back(std::move(skt));
            }
            
            m_sockets.interrupt();
        }        
        
        bool running() const {
            return m_running.load(std::memory_order_relaxed);
        }
        
        void stop() {
            m_running = false;
            m_sockets.interrupt();
        }
        
        std::function<void()> on_interrupt;
        void interrupt() {
            m_sockets.interrupt();
        }
    private:   
        void clear_backlog() {
            std::lock_guard<std::mutex> lg(m_mutex);
            
            for (auto& skt : m_backlog) {
                item new_item(std::move(skt));
                m_sockets.add_socket(std::move(new_item));
            }
            
            m_backlog.clear();
        }        
        
        struct item { 
            item(skt_T&& s) : skt(std::move(s)) {}
            
            skt_T skt;
            arg_T arg;
            
            int get_socket() {
                return skt.get_socket();
            }
        };        
                
        std::atomic<bool> m_running;
        std::mutex m_mutex;
        std::thread m_thread;   
        std::vector<skt_T> m_backlog;
        
        network::socket_container<item> m_sockets;
    };
}
