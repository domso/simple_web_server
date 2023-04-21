#pragma once

#include <vector>
#include <functional>
#include "sys/epoll.h"
#include "sys/eventfd.h"
#include <assert.h>
#include <unordered_map>
#include <mutex>
#include <optional>
#include "wait_ops.h"

namespace network {    
    template<typename T>
    class socket_container {
    public: 
        socket_container(const size_t buffer_size) : m_events(buffer_size) {
            m_epfd = epoll_create1(0);
            m_eventfd = eventfd(0, 0);
            assert(m_epfd >= 0 && m_eventfd >= 0);
            
            add_fd(m_eventfd, nullptr, false);            
        }
        
        ~socket_container() {
            close(m_eventfd);
            close(m_epfd);
        }
        
        socket_container(const socket_container&) = delete;
        socket_container(const socket_container&&) = delete;        
        void operator=(const socket_container&) = delete;
        void operator=(const socket_container&&) = delete;
        
        void add_socket(T&& skt) {        
            auto new_skt = new T(std::move(skt));
            add_fd(new_skt->get_fd(), reinterpret_cast<void*>(new_skt), true);
            m_item_map.insert({new_skt->get_id(), new_skt});
        }
                
        bool wait(std::function<wait_ops(T& skt)> call, const int timeout) {
            int result = epoll_wait(m_epfd, m_events.data(), m_events.size(), timeout);
            if (result >= 0) {
                for (int i = 0; i < result; i++) {                                        
                    T* skt = static_cast<T*>(m_events[i].data.ptr);
                    if (skt == nullptr) {
                        uint64_t buf;
                        assert(read(m_eventfd, &buf, sizeof(buf)) != -1);
                    } else {
                        call_callback(skt, call);                    
                    }                
                }       
                
                result += clear_force_list(call);
            }
            
            return result > 0;
        }        
        
        void interrupt() {
            uint64_t buf = 1;
            assert(write(m_eventfd, &buf, sizeof(buf)) != -1);
        }        
        
        void force_call(const uint64_t id) {
            std::lock_guard<std::mutex> lg(m_mutex);
            m_force_call_list.push_back(id);
            interrupt();
        }
    private:         
        void call_callback(T* skt, std::function<wait_ops(T& skt)>& call) {
            int fd = skt->get_fd();
            auto op = call(*skt);
            
            switch (op) {
                case wait_write: {
                    set_rw(fd, skt, false, true);  
                    break;
                }
                case wait_read: {
                    set_rw(fd, skt, true, false);   
                    break;                         
                }
                case wait_read_write: {
                    set_rw(fd, skt, true, true);
                    break;
                }
                case remove: {
                    remove_fd(fd);
                    m_item_map.erase(skt->get_id());
                    delete skt;  
                    break;                            
                }
            }
        }
        
        std::vector<uint64_t> get_force_list() {
            std::lock_guard<std::mutex> lg(m_mutex);
            return std::move(m_force_call_list);            
        }
        
        int clear_force_list(std::function<wait_ops(T& skt)>& call) {
            std::vector<uint64_t> list;
            int result = 0;
            
            {                
                std::lock_guard<std::mutex> lg(m_mutex);
                list = std::move(m_force_call_list);      
            }
                        
            for (const auto c : list) {
                auto find = m_item_map.find(c);
                if (find != m_item_map.end()) {                    
                    call_callback(find->second, call);
                    result++;
                }
            }
                            
            return result;
        }
        
        void add_fd(int fd, void* ptr, const bool write) {
            epoll_event event;            
            event.events = EPOLLIN | EPOLLPRI;
            if (write) {
                event.events |= EPOLLOUT;
            }
            event.data.ptr = ptr;
                        
            int result = epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &event);
                        
            assert(result == 0);
        }         
        
        void set_rw(int fd, void* ptr, const bool read, const bool write) {
            epoll_event event;            
            event.events = EPOLLPRI;
            
            if (read) {
                event.events |= EPOLLIN;
            }
            
            if (write) {
                event.events |= EPOLLOUT;
            }
            event.data.ptr = ptr;
                        
            int result = epoll_ctl(m_epfd, EPOLL_CTL_MOD, fd, &event);
                        
            assert(result == 0);            
        }
        
        void remove_fd(int fd) {            
            int result = epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, nullptr);
            assert(result == 0); 
        }
        
        std::vector<epoll_event> m_events;
        std::unordered_map<uint64_t, T*> m_item_map;
        std::mutex m_mutex;
        std::vector<uint64_t> m_force_call_list;
        
        int m_epfd;
        int m_eventfd;
    };  
}
