#pragma once

#include <functional>
#include <array>
#include <stdint.h>

#include "network/memory_region.h"

namespace web_server::modules::web_socket {
    class frame_encoder {
    public:              
        frame_encoder(const size_t max_fragment_size);
        
        void pack_data(
            const network::memory_region region,
            const std::function<void(const network::memory_region)> callback
        );
    private:   
        void pack_fragment(
            const network::memory_region region,
            const bool last, 
            const std::function<void(const network::memory_region)> callback
        );      
        
        size_t m_max_fragment_size;
        std::array<uint8_t, 14> m_frame_data;
    };
}
