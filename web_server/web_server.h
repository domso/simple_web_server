#pragma once

#include <memory>
#include <string>
#include <optional>
#include "signal.h"

#include "network/tcp_socket.h"
#include "network/ssl_context.h"
#include "network/ssl_connection.h"
#include "network/status.h"
#include "worker.h"

#include "util/signal_interrupt.h"

#include "unique_context.h"
#include "network_interface.h"

namespace web_server {

template<typename T_config, typename T, typename... Ts>
class web_server {
public:
    bool init() {
        util::logger::init<T_config>();
        util::logger::log_status("Initiate server");
        if (!m_socket.accept_on(T_config::template get_value<"port">(), T_config::template get_value<"max_pending">())) {
            util::logger::log_error("Could not open socket");
            return false;
        }
        if (!m_ssl_context.init(
                std::string(T_config::template get_value<"certificate">().data()), 
                std::string(T_config::template get_value<"key">().data()), 
                std::string(T_config::template get_value<"password">().data())
            )) {
            util::logger::log_error("Could not open credentials");
            return false;
        }
        
        return true;
    }

    bool run() {
        util::logger::log_status("Start server");
        signal(SIGPIPE, SIG_IGN);
        util::signal_interrupt itr([this](){
           util::logger::log_status("Interrupt");
           m_socket.close_socket(); 
        });
            
        start_http_worker();
        start_bouncer_worker();

        while (auto tcp_connection = m_socket.accept_connection()) {
            util::logger::log_status("New TCP connection from " + tcp_connection->get_addr().ip() + ":" + std::to_string(tcp_connection->get_addr().port()));
            tcp_connection->set_blocking(false);
            auto ssl_connection = m_ssl_context.convert(std::move(*tcp_connection));
            m_bouncer_worker->add_socket(std::move(ssl_connection));                           
        }
            
        util::logger::log_status("Stop server");
        return true;
    }

    template<typename Treq>
    auto& get_module() {
        return m_net_if.template get_module<Treq>();
    }
    template<typename Treq>
    const auto& get_module() const {
        return m_net_if.template get_module<Treq>();
    }
private:    
    void start_http_worker() {
        typedef worker<network::ssl_connection<network::ipv4_addr>, ::web_server::unique_context<T, Ts...>> worker_type;
        
        for (size_t i = 0; i < T_config::template get_value<"num_worker">(); i++) {
            auto worker_instance = std::make_unique<worker_type>(T_config::template get_value<"worker_poll_timeout">(), T_config::template get_value<"worker_poll_buffer_size">());
            worker_instance->start([&](network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<unique_context<T, Ts...>>& context, network::socket_container_notifier& notifier){
                return m_net_if.update(conn, context, notifier);
            });
            m_http_workers.push_back(std::move(worker_instance));
        }
    }

    void start_bouncer_worker() {
        typedef worker<network::ssl_connection<network::ipv4_addr>, void> worker_type;
        
        m_select_worker = 0;
        m_bouncer_worker = std::make_unique<worker_type>(T_config::template get_value<"accept_poll_timeout">(), T_config::template get_value<"accept_poll_buffer_size">());
        m_bouncer_worker->start([&](network::ssl_connection<network::ipv4_addr>& conn, std::shared_ptr<void>&, network::socket_container_notifier& notifier){
            auto id = m_select_worker++;
            if (m_select_worker == m_http_workers.size()) {
                m_select_worker = 0;
            }
            return m_net_if.ssl_accept(conn, *m_http_workers[id]);        
        });
    }

    network::tcp_socket<network::ipv4_addr> m_socket;
    network::ssl_context<network::ipv4_addr> m_ssl_context;

    std::vector<std::unique_ptr<worker<network::ssl_connection<network::ipv4_addr>, unique_context<T, Ts...>>>> m_http_workers;
    std::unique_ptr<worker<network::ssl_connection<network::ipv4_addr>, void>> m_bouncer_worker;
    size_t m_select_worker;
    
    network_interface<T_config, T, Ts...> m_net_if;
};
}
