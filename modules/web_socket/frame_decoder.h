#pragma once

#include <functional>
#include <vector>
#include <stdint.h>
#include <cstring>

#include "network/memory_region.h"

namespace web_server::modules::web_socket {
    class frame_decoder {
    public:              
        frame_decoder() {};                
        size_t unpack_data(const network::memory_region region, const std::function<void(const network::memory_region, const uint8_t)> callback) const;
    private:   
        void mask_data(network::memory_region data_region, const network::memory_region mask_region) const;        
    };
}

