#include "handle.h"

web_server::native::handle::handle(const std::shared_ptr<unique_context>& context) : m_context(context) {}        
web_server::native::handle::handle(const handle& handle) : m_context(handle.m_context) {}        
web_server::native::handle::handle(handle&& handle) : m_context(std::move(handle.m_context)) {}  

void web_server::native::handle::operator=(const web_server::native::handle& handle) {m_context = handle.m_context;}
void web_server::native::handle::operator=(web_server::native::handle&& handle) {m_context = std::move(handle.m_context);}

std::function<size_t(const network::memory_region)>& web_server::native::handle::on_recv() {
    return m_context->native_if.on_recv;
}

std::function<void()>& web_server::native::handle::on_close() {
    return m_context->native_if.on_close;
}

void web_server::native::handle::interrupt() {
    if (m_context->force_call) {
        m_context->force_call();
    }
}

void web_server::native::handle::send(std::vector<char>&& vec) {
    m_context->native_if.send(std::forward<std::vector<char>>(vec));
}

void web_server::native::handle::send(network::memory_region data) {
    m_context->native_if.send(data);
}

bool web_server::native::handle::is_valid() const {
    return m_context->is_valid;
}

void web_server::native::handle::close() {
    m_context->is_valid = false;
}
