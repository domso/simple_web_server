#include "native_interface.h"

web_server::native_interface::~native_interface() {
    if (on_close) {
        on_close();
    }
}

void web_server::native_interface::send(std::vector<char>&& vec) {
    std::unique_lock<std::mutex> ul(m_mutex);
    m_data.push_back(std::move(vec));
}

void web_server::native_interface::push_into(std::vector<char> data) {
    std::unique_lock<std::mutex> ul(m_mutex);
    for (auto& v : m_data) {
        data.insert(data.end(), v.begin(), v.end());
    }
    
    m_data.clear();
} 
