#include "frame_encoder.h"

web_server::modules::web_socket::frame_encoder::frame_encoder(const size_t max_fragment_size) : m_max_fragment_size(max_fragment_size) {};

void web_server::modules::web_socket::frame_encoder::pack_data(
    const network::memory_region_view region,
    const std::function<void(const network::memory_region_view)> callback
) {                
    for (size_t start = 0; start < region.size(); start += m_max_fragment_size) {
        pack_fragment(
            region.splice(start, m_max_fragment_size), 
            start == 0,
            (start + m_max_fragment_size) >= region.size(),
            callback
        );                    
    }    
}

void web_server::modules::web_socket::frame_encoder::pack_fragment(
    const network::memory_region_view region,
    const bool first, 
    const bool last, 
    const std::function<void(const network::memory_region_view)> callback
) {
    size_t n = region.size();
                
    m_frame_data[0] = (static_cast<uint32_t>(last) << 7) | first; 
    size_t header_size = 0;
    
    if (n < 126) {
        m_frame_data[1] = n;
        header_size = 2;
    } else if (126 <= n && n < 0x10000) {
        m_frame_data[1] = 126;
        m_frame_data[2] = (n >> 8) & 0xFF;
        m_frame_data[3] = (n >> 0) & 0xFF;
        header_size = 4; 
    } else if (0x10000 <= n) {
        m_frame_data[1] = 127;
        m_frame_data[2] = (n >> 56) & 0xFF;
        m_frame_data[3] = (n >> 48) & 0xFF;
        m_frame_data[4] = (n >> 40) & 0xFF;
        m_frame_data[5] = (n >> 32) & 0xFF;
        m_frame_data[6] = (n >> 24) & 0xFF;
        m_frame_data[7] = (n >> 16) & 0xFF;
        m_frame_data[8] = (n >> 8)  & 0xFF;
        m_frame_data[9] = (n >> 0)  & 0xFF;
        header_size = 10; 
    }
    
    callback(network::memory_region_view(m_frame_data.data(), header_size));
    callback(region);    
}        
