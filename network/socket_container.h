#pragma once

#include <vector>
#include <functional>
#include "sys/epoll.h"
#include "sys/eventfd.h"
#include <assert.h>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <optional>
#include "wait_ops.h"

namespace network {
    struct socket_container_external_interrupt_context {
        std::mutex mutex;
        std::vector<uint64_t> call_list;
        int eventfd;

        void add_to_call_list(const uint64_t id) {
            std::lock_guard<std::mutex> lg(mutex);
            call_list.push_back(id);
        }

        void interrupt() {
            uint64_t buf = 1;
            assert(write(eventfd, &buf, sizeof(buf)) != -1);
        }
    };
    struct socket_container_notifier {
        std::shared_ptr<socket_container_external_interrupt_context> context;
        uint64_t id;

        void notify() {
            if (context) {
                context->add_to_call_list(id);
                context->interrupt();
            }
        }
    };

    template<typename T>
    class socket_container {
    public:
        socket_container(const size_t buffer_size) : m_events(buffer_size) {
            m_epfd = epoll_create1(0);
            m_external_interrupt_context = std::make_shared<socket_container_external_interrupt_context>();
            m_external_interrupt_context->eventfd = eventfd(0, 0);
            assert(m_epfd >= 0 && m_external_interrupt_context->eventfd >= 0);

            add_fd(m_external_interrupt_context->eventfd, nullptr, false);
        }

        ~socket_container() {
            for (auto& [key, value] : m_item_map) {
                delete value;
            }
            m_item_map.clear();
            while (m_external_interrupt_context.use_count() > 1) {}
            close(m_external_interrupt_context->eventfd);
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
            assert(result >= 0);

            for (int i = 0; i < result; i++) {
                T* skt = static_cast<T*>(m_events[i].data.ptr);
                if (skt == nullptr) {
                    uint64_t buf;
                    assert(read(m_external_interrupt_context->eventfd, &buf, sizeof(buf)) != -1);
                } else {
                    call_callback(skt, call);
                }
            }

            result += clear_notify_list(call);

            return result > 0;
        }

        void interrupt() {
            uint64_t buf = 1;
            assert(write(m_external_interrupt_context->eventfd, &buf, sizeof(buf)) != -1);
        }

        socket_container_notifier notifier(const uint64_t id) {
            socket_container_notifier notifier;
            notifier.context = m_external_interrupt_context;
            notifier.id = id;

            return notifier;
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

        std::vector<uint64_t> get_notify_list() {
            std::lock_guard<std::mutex> lg(m_external_interrupt_context->mutex);
            return std::move(m_external_interrupt_context->call_list);
        }

        int clear_notify_list(std::function<wait_ops(T& skt)>& call) {
            std::vector<uint64_t> list = get_notify_list();
            int result = 0;

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

        int m_epfd;
        std::shared_ptr<socket_container_external_interrupt_context> m_external_interrupt_context;
    };
}
