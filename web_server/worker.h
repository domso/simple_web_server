#pragma once

#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include <unordered_map>
#include <memory>
#include <type_traits>

#include "network/socket_container.h"

namespace web_server {
    template<typename skt_T, typename arg_T>
    class worker {
    public:
        worker(std::function<network::wait_ops(skt_T&, std::shared_ptr<arg_T>&)> call, const size_t poll_timeout, const size_t poll_buffer_size) : m_running(true), m_current_id(1), m_sockets(poll_buffer_size){            
            m_thread = std::thread([call, poll_timeout, this]() {                            
                while(m_running.load(std::memory_order_relaxed)) {
                    clear_backlog();

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
    private:   
        void clear_backlog() {
            std::lock_guard<std::mutex> lg(m_mutex);
            
            for (auto& skt : m_backlog) {
                item new_item(std::move(skt), m_current_id++);
                m_sockets.add_socket(std::move(new_item));
            }
            
            m_backlog.clear();
        }
        
        struct item { 
            item(skt_T&& s, uint64_t id) : skt(std::move(s)) {
                if constexpr (!std::is_same<arg_T, void>::value) {
                    arg = std::make_shared<arg_T>();
                    arg->id = id;                    
                }                
            }
            
            skt_T skt;
            std::shared_ptr<arg_T> arg;
            
            int get_socket() {
                return skt.get_socket();
            }
            
            uint64_t get_id() {
                if constexpr (!std::is_same<arg_T, void>::value) {
                    return arg->id;
                }
                
                return 0;
            }
            
            void close() {
                if constexpr (!std::is_same<arg_T, void>::value) {
                    arg->valid = false;
                }               
            }
        };        
                
        std::atomic<bool> m_running;
        std::mutex m_mutex;
        std::thread m_thread;   
        std::vector<skt_T> m_backlog;
        uint64_t m_current_id;
        
        network::socket_container<item> m_sockets;
    };
}
