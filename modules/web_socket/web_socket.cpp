#include "web_socket.h"

namespace web_server::modules::web_socket {
    void web_socket::init(network::socket_container_notifier& notifier) {
        m_internal = std::make_shared<web_socket_data>();
        m_internal->notifier = notifier;
    }
    void web_socket::send(network::memory_region_view send_data) {
        if (send_data.size() == 0) {
            return;
        }
        std::vector<char> encoded_data;
        m_internal->encoder.pack_data(send_data, [&](const network::memory_region_view region) {
            region.push_back_into(encoded_data);
        });

        {
            std::unique_lock ul(m_internal->mutex);
            m_internal->send_buffer.insert(m_internal->send_buffer.end(), encoded_data.begin(), encoded_data.end());
        }

        m_internal->notifier.notify();
    }
    void web_socket::update(std::vector<char>& buffer) {
        std::unique_lock ul(m_internal->mutex);
        buffer.insert(buffer.end(), m_internal->send_buffer.begin(), m_internal->send_buffer.end());
        m_internal->send_buffer.clear();
    }
    void web_socket::close() {
        m_internal->valid = false;
    }
    bool web_socket::status() const {
        return m_internal->valid;
    }
}
