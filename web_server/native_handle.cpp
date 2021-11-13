#include "native_handle.h"

web_server::native_handle::native_handle(std::shared_ptr<unique_context>& context) : m_context(context) {}        
web_server::native_handle::native_handle(const native_handle& handle) : m_context(handle.m_context) {}        
web_server::native_handle::native_handle(native_handle&& handle) : m_context(std::move(handle.m_context)) {}  

void web_server::native_handle::operator=(const web_server::native_handle& handle) {m_context = handle.m_context;}
void web_server::native_handle::operator=(web_server::native_handle&& handle) {m_context = std::move(handle.m_context);}

std::function<bool(const network::memory_region)>& web_server::native_handle::on_recv() {
        return m_context->native_caller.on_recv;
}

std::function<void()>& web_server::native_handle::on_close() {
    return m_context->native_caller.on_close;
}

void web_server::native_handle::send(std::vector<char>&& vec) {
    m_context->native_caller.send(std::forward<std::vector<char>>(vec));
}
