#pragma once

#include <memory>

#include "unique_context.h"
#include "native_interface.h"

namespace web_server {
    class native_handle {
    public:
        native_handle(std::shared_ptr<unique_context>& context);   
        native_handle(const native_handle& handle);     
        native_handle(native_handle&& handle);
        void operator=(const native_handle& handle);
        void operator=(native_handle&& handle);
                
        std::function<bool(const network::memory_region)>& on_recv();        
        std::function<void()>& on_close();
        
        void send(std::vector<char>&& vec);
    private:
        std::shared_ptr<unique_context> m_context;
    }; 
}
