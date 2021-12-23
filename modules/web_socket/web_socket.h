#pragma once

#include <functional>

#include "network/memory_region.h"
#include "web_server/native/handle.h"

namespace web_server::modules::web_socket {
    class web_socket {
    public:
        web_socket(const native::handle& handle);              
        web_socket(const web_socket& s);
        web_socket(web_socket&& s);
        
        void send(network::memory_region send_data);
        
        void set_on_recv(std::function<void(const network::memory_region, web_socket& socket)> call);                
        void set_on_close(std::function<void()> call);
        bool is_valid() const;
        void close();
    private:   
        bool m_init;
        native::handle m_handle;                  
    };
}
