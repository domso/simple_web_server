#pragma once

#include <vector>
#include "sys/epoll.h"
#include "sys/eventfd.h"
#include <assert.h>
#include "wait_ops.h"

namespace network {
    template<typename T>
    class epoll_server {
    public:
        socket_container(const size_t buffer_size) : m_events(buffer_size) {
            m_epfd = epoll_create1(0);
            assert(m_epfd >= 0);
        }

        ~socket_container() {
            close(m_epfd);
        }

        socket_container(const socket_container&) = delete;
        socket_container(const socket_container&&) = delete;
        void operator=(const socket_container&) = delete;
        void operator=(const socket_container&&) = delete;

        void add_socket(T&& skt) {
            auto new_skt = new T(std::move(skt));
            {
                std::unique_lock ul(m_mutex);
                add_fd(new_skt->get_fd(), reinterpret_cast<void*>(new_skt), true);
                m_item_queue.push_back({new_skt->get_id(), new_skt});
            }
        }

        template<typename T_call>
        bool wait(const T_call& call, const int timeout) {
            int result = epoll_wait(m_epfd, m_events.data(), m_events.size(), timeout);
            assert(result >= 0);

            for (int i = 0; i < result; i++) {
                T* data = static_cast<T*>(m_events[i].data.ptr);
                call_callback(data, call);
            }

            return result > 0;
        }
    private:
        template<typename T_call>
        void call_callback(T* data, const T_call& call) {
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
                    update_item_map();
                    m_item_map.erase(skt->get_id());
                    delete skt;
                    break;
                }
            }
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
            assert(result == 0 || errno == EBADF);
        }

        std::vector<epoll_event> m_events;
        int m_epfd;
    };
}
