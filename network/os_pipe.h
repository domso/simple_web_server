#pragma once

#include <array>
#include <unistd.h>
#include <fcntl.h>
//#include <netinet/tcp.h>

#include <iostream>

namespace network {
    class os_pipe {
    public:
        os_pipe() {
            auto e = pipe2(m_fds.data(), 0);
            if (e != 0) {
                m_fds = {0, 0};
            }
        }
        ~os_pipe() {
            if (is_valid()) {
                close_pipe();
            }
        }
        os_pipe(const os_pipe&) = delete;
        os_pipe(os_pipe&& other) {
            m_fds = other.m_fds;
            other.m_fds = {0, 0};
        }
        void operator=(const os_pipe&) = delete;
        void operator=(os_pipe&& other) {
            m_fds = other.m_fds;
            other.m_fds = {0, 0};
        }
        bool is_valid() const {
            return m_fds != std::array<int, 2>({0, 0});
        }
        void close_pipe() {
            close(m_fds[0]);
            close(m_fds[1]);
            m_fds = {0, 0};
        }
        int write_fd() const {
            return m_fds[1];
        }
        int read_fd() const {
            return m_fds[0];
        }
        template<typename T>
        bool read_from(T& s) {
            int e = splice(s.get_socket(), nullptr, write_fd(), nullptr, 512, SPLICE_F_MOVE);

            if (e <= 0) {
                close_pipe();
            }

            return e > 0;
        }
        template<typename T>
        bool write_to(T& s) {
            int e = splice(read_fd(), nullptr, s.get_socket(), nullptr, 512, SPLICE_F_MOVE);

            if (e <= 0) {
                close_pipe();
            }

            return e > 0;
        }
    private:
        std::array<int, 2> m_fds;
    };
}
