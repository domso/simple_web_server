#pragma once

#include <memory>

#include "web_server/unique_context.h"
#include "send_recv_interface.h"

namespace web_server::native {
    class handle {
    public:
        handle(const std::shared_ptr<unique_context>& context);   
        handle(const handle& handle);     
        handle(handle&& handle);
        void operator=(const handle& handle);
        void operator=(handle&& handle);
                
        std::function<size_t(const network::memory_region)>& on_recv();        
        std::function<void()>& on_close();
        
        void interrupt();
        
        void send(std::vector<char>&& vec);
        void send(network::memory_region data);
        bool is_valid() const;
        void close();
    private:
        std::shared_ptr<unique_context> m_context;
    }; 
}
