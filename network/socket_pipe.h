#pragma once

#include "os_pipe.h"

namespace network {
    class socket_pipe {
    public:
        template<typename T_src, typename T_dest>
        bool forward_from_to(T_src& src, T_dest& dest) {
            return m_pipe.read_from(src) && m_pipe.write_to(dest);
        }
    private:
        os_pipe m_pipe;
    };
}

