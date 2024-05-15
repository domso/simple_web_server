#pragma once

#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>
#include <unordered_map>
#include <memory>
#include <type_traits>
#include "signal.h"

#include "util/logger.h"

#include "network/socket_container.h"

namespace web_server {
    template<typename skt_T, typename arg_T>
    class worker {
    public:
        worker(const size_t poll_timeout, const size_t poll_buffer_size) : m_running(false), m_poll_timeout(poll_timeout), m_sockets(poll_buffer_size) {}

        template<typename call_T> 
        void start(const call_T& call) {            
            m_running = true;

            m_thread = std::thread([call, this]() {
                while(m_running.load(std::memory_order_relaxed)) {
                    clear_backlog();

                    m_sockets.wait([&](skt_T& socket, std::shared_ptr<arg_T>& data, network::socket_container_notifier& notifier) {
                        return call(socket, data, notifier);
                    }, m_poll_timeout);
                }                
            });            
        }
        
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
                m_sockets.add_socket(std::move(skt), std::make_shared<arg_T>());
            }
            
            m_backlog.clear();
        }
        
        std::atomic<bool> m_running;
        std::mutex m_mutex;
        std::thread m_thread;   
        std::vector<skt_T> m_backlog;
        size_t m_poll_timeout;
        
        network::socket_container<skt_T, std::shared_ptr<arg_T>> m_sockets;
    };
}
