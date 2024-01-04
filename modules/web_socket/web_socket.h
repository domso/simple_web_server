#pragma once

#include <atomic>
#include <mutex>
#include <memory>

#include "frame_decoder.h" 
#include "frame_encoder.h"

#include "network/memory_region.h"
#include "network/memory_region_view.h"
#include "network/socket_container.h"

namespace web_server::modules::web_socket {
    struct web_socket_data {
        frame_encoder encoder = 1024;
        frame_decoder decoder;
        std::vector<char> recv_buffer;
        network::socket_container_notifier notifier;  
        std::atomic<bool> valid = true;

        std::mutex mutex;
        std::vector<char> send_buffer;
    };

    class web_socket {
    public:
        void init(network::socket_container_notifier& notifier);
        
        template<typename Tcall>
        size_t recv(network::memory_region read_region, const Tcall& call) {
            return m_internal->decoder.unpack_data(read_region, [&](const network::memory_region_view region, const uint8_t header) {                    
                region.push_back_into(m_internal->recv_buffer);
                
                if ((header & 0xF) == 8) {
                    close();
                    return;
                }
                
                if ((header & 128) > 0) {   
                    network::memory_region_view decoded_data;
                    decoded_data.use(m_internal->recv_buffer);
                    call(decoded_data, *this);
                    m_internal->recv_buffer.clear();
                }  
            });
        }
        void send(network::memory_region_view send_data);
        void update(std::vector<char>& buffer);
        void close();
        bool status() const;
    private:
        std::shared_ptr<web_socket_data> m_internal;
    };
}
