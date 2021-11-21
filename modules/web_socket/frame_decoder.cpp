#include "frame_decoder.h"

size_t web_server::modules::web_socket::frame_decoder::unpack_data(
    const network::memory_region region,
    const std::function<void(const network::memory_region, const uint8_t)> callback
) const {  
    if (region.size() < 2) {
        return 0;
    } else {
        uint64_t header_length = 0;
        size_t data_length = 0;
        
        uint8_t byte_len = region.data()[1] & 127;
        uint8_t use_mask = (region.data()[1] & 128) >> 5;
        
        if (byte_len < 126) {
            data_length = byte_len;
            header_length = 2;
        } else if (byte_len == 126 && region.size() >= 4) {
            data_length = (region.data()[2] << 8) | region.data()[3];
            header_length = 4;
        } else if (byte_len == 127 && region.size() >= 10) {
            data_length = (static_cast<size_t>(region.data()[2]) << 56) | 
                        (static_cast<size_t>(region.data()[3]) << 48) |
                        (static_cast<size_t>(region.data()[4]) << 40) | 
                        (static_cast<size_t>(region.data()[5]) << 32) | 
                        (static_cast<size_t>(region.data()[6]) << 24) | 
                        (static_cast<size_t>(region.data()[7]) << 16) | 
                        (static_cast<size_t>(region.data()[8]) << 8)  |
                        static_cast<size_t>(region.data()[9]);
            header_length = 10;
        } else {
            return 0;
        }
        header_length += use_mask;   
                            
        if (header_length + data_length <= region.size()) {                        
            mask_data(region.splice(header_length, data_length), region.splice(header_length - use_mask, 4));
            callback(region.splice(header_length, data_length), region.data()[0]);
            return header_length + data_length;
        }
        
        return 0;
    }
}
void web_server::modules::web_socket::frame_decoder::mask_data(network::memory_region data_region, const network::memory_region mask_region) const {
    if (!data_region.overlap(mask_region)) {
        int mode = 0;
        
        for (auto it = data_region.begin(); it != data_region.end(); it++) {                        
            *it ^= mask_region.data()[mode];                      
            mode = (mode + 1) % 4;
        }                    
    }
}
