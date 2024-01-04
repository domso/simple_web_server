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
        worker(const size_t poll_timeout, const size_t poll_buffer_size) : m_running(false), m_current_id(0), m_poll_timeout(poll_timeout), m_sockets(poll_buffer_size) {}

        template<typename call_T> 
        void start(const call_T& call) {            
            m_running = true;
            m_current_id = 1;

            m_thread = std::thread([call, this]() {
                while(m_running.load(std::memory_order_relaxed)) {
                    clear_backlog();

                    m_sockets.wait([&](item& c) {
                        return call(c.get_skt(), c.get_arg());
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
                auto id = m_current_id++;
                auto notifier = m_sockets.notifier(id);
                item new_item(std::move(skt), id, notifier);
                m_sockets.add_socket(std::move(new_item));
            }
            
            m_backlog.clear();
        }
        
        class item { 
        public:
            item(skt_T&& s, const uint64_t id, network::socket_container_notifier& notifier) : m_skt(std::move(s)) {
                if constexpr (!std::is_same<arg_T, void>::value) {
                    m_arg = std::make_shared<arg_T>();
                    m_arg->id = id;      
                    m_arg->notifier = notifier;
                    m_arg->name = m_skt.to_string();
                    util::logger::log_debug("Create context of " + m_arg->name);
                }                
            }
            item(item&& move) : m_skt(std::move(move.m_skt)), m_arg(std::move(move.m_arg)) {}
             
            int get_fd() {
                return m_skt.get_socket();
            }
            
            uint64_t get_id() const {
                if constexpr (!std::is_same<arg_T, void>::value) {
                    return m_arg->id;
                }
                
                return 0;
            }
            
            skt_T& get_skt() {
                return m_skt;
            }
            
            std::shared_ptr<arg_T>& get_arg() {
                return m_arg;
            }
        private:
            skt_T m_skt;
            std::shared_ptr<arg_T> m_arg;
        };        
                
        std::atomic<bool> m_running;
        std::mutex m_mutex;
        std::thread m_thread;   
        std::vector<skt_T> m_backlog;
        uint64_t m_current_id;
        size_t m_poll_timeout;
        
        network::socket_container<item> m_sockets;
    };
}
