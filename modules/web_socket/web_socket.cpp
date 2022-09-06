#include "web_socket.h"

#include <vector>
#include "assert.h"

#include "frame_decoder.h" 
#include "frame_encoder.h"

web_server::modules::web_socket::web_socket::web_socket(const native::handle& handle) : m_init(true), m_handle(handle) {}
web_server::modules::web_socket::web_socket::web_socket(const web_socket& s) : m_init(false), m_handle(s.m_handle) {}
web_server::modules::web_socket::web_socket::web_socket(web_socket&& s) : m_init(false), m_handle(std::move(s.m_handle)) {};

void web_server::modules::web_socket::web_socket::send(network::memory_region_view send_data) {
    std::vector<char> encoded_data;
    frame_encoder encoder = 1024;  
    encoder.pack_data(send_data, [&](const network::memory_region_view region) {
        region.push_back_into(encoded_data);
    });
    m_handle.send(std::move(encoded_data));
    m_handle.interrupt();
}

void web_server::modules::web_socket::web_socket::set_on_recv_byte(std::function<void(const network::memory_region_view, web_socket& socket)> call) {
    assert(m_init);
    m_handle.on_recv() = [call, decoder = frame_decoder(), recv_data = std::vector<char>(), socket = web_server::modules::web_socket::web_socket(*this)] (network::memory_region region) mutable {                            
        return decoder.unpack_data(region, [&](const network::memory_region_view region, const uint8_t header) {                    
            region.push_back_into(recv_data);
            
            if ((header & 0xF) == 8) {
                socket.close();
                return;
            }
            
            if ((header & 128) > 0) {   
                network::memory_region_view decoded_data;
                decoded_data.use(recv_data);
                call(decoded_data, socket);
                recv_data.clear();
            }  
        });
    };
}

void web_server::modules::web_socket::web_socket::set_on_close(std::function<void()> call) {          
    assert(m_init);
    m_handle.on_close() = call;
}

bool web_server::modules::web_socket::web_socket::is_valid() const {
    return m_handle.is_valid();
}

void web_server::modules::web_socket::web_socket::close() {
    m_handle.close();
    m_handle.interrupt();
}
