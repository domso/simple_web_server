#include "send_recv_interface.h"
#include "util/logger.h"
web_server::native::send_recv_interface::~send_recv_interface() {
    if (on_close) {
        on_close();
    }
}

void web_server::native::send_recv_interface::send(std::vector<char>&& vec) {
    std::unique_lock<std::mutex> ul(m_mutex);
    m_data.push_back(std::move(vec));
}

void web_server::native::send_recv_interface::send(network::memory_region data) {
    std::unique_lock<std::mutex> ul(m_mutex);
    m_data.push_back({});
    data.push_back_into(*m_data.rbegin());
}

void web_server::native::send_recv_interface::push_into(std::vector<char>& data) {
    std::unique_lock<std::mutex> ul(m_mutex);
    for (auto& v : m_data) {
        data.insert(data.end(), v.begin(), v.end());
    }
    
    m_data.clear();
} 

void web_server::native::send_recv_interface::close() {  
    if (on_close) {  
        on_close();
    }   
    
    on_recv = [](const network::memory_region){return 0;};
    on_close = [](){};    
}
