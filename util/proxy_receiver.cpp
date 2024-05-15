#include "network/tcp_connection.h"
#include "network/tcp_socket.h"
#include "network/socket_pipe.h"
#include "network/socket_container.h"
#include "sys/epoll.h"
#include "signal.h"

#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>
#include <condition_variable>

#include "command_line_argument_parser.h"
#include "string_to_int.h"

struct connection_data {
    network::socket_container_notifier notifier;

    std::vector<char> send_data;
    std::vector<char> recv_buffer;
    std::vector<char> recv_data;

    bool is_control;
    bool is_toclose = false;
};

int main(int argc, char **argv) {
    web_server::command_line_argument_parser cmd(argc, argv);
    network::tcp_socket<network::ipv4_addr> public_socket;
    network::tcp_socket<network::ipv4_addr> ctrl_socket;

    cmd.add_help_text("Proxy Receiver");
    cmd.add_required(
        "Port of the control channel",
        [&](const std::string& port) {
            if (auto maybe_port = web_server::string_to_int<uint32_t>(port)) {
                return ctrl_socket.accept_on(*maybe_port, 10);
            }   
            return false;
        },  
        "--control"
    );  
    cmd.add_required(
        "Port of the public channel",
        [&](const std::string& port) {
            if (auto maybe_port = web_server::string_to_int<uint32_t>(port)) {
                return public_socket.accept_on(*maybe_port, 10);
            }   
            return false;
        },  
        "--public"
    );  

    if (!cmd.parse_arguments()) {
        return -1;
    }

    ctrl_socket.accept_on(6590, 10);
    public_socket.accept_on(6585, 10);

    signal(SIGPIPE, SIG_IGN);

    network::socket_container<network::tcp_connection<network::ipv4_addr>, std::shared_ptr<connection_data>> sockets(10);
    int16_t ctrl_port;

    std::thread ctrl_thread([&]() mutable {
        while (auto ctrl_connection = ctrl_socket.accept_connection()) {
            auto d = std::make_shared<connection_data>();
            d->is_control = true;
            ctrl_port = ctrl_connection->get_addr().port();
            ctrl_connection->set_blocking(false);
            //std::cout << "new ctrl " << ctrl_connection->get_addr().port() << std::endl;
            sockets.add_socket(std::move(*ctrl_connection), d, true);
        }
    });

    std::thread accept_thread([&]() mutable {
        while (auto public_connection = public_socket.accept_connection()) {
            auto d = std::make_shared<connection_data>();
            d->is_control = false;
            public_connection->set_blocking(false);
            std::cout << "new public " << public_connection->get_addr().port() << std::endl;
            sockets.add_socket(std::move(*public_connection), d, true);
        }
    });

    std::unordered_map<uint16_t, std::shared_ptr<connection_data>> port_map;

    while (true) {
        sockets.wait([&](network::tcp_connection<network::ipv4_addr>& connection, std::shared_ptr<connection_data>& data, network::socket_container_notifier& notifier) {
            auto port = connection.get_addr().port();
            //std::cout << "woke on " << port << std::endl;
            if (!port_map.contains(port)) {
                data->notifier = notifier;                
                port_map[port] = data;
            }

            if (data->is_toclose) {
                std::cout << "close on " << port << std::endl;
                port_map.erase(port);
                return network::wait_ops::remove;
            }

            if (!data->send_data.empty()) {
                network::memory_region send_region;
                send_region.use(data->send_data);
                auto [result, n] = connection.send_data(send_region);
                //std::cout << "send on " << port << " " << n << " " << send_region.size() << std::endl;

                if (result) {
                    if (n == 0) {
                        return network::wait_ops::wait_write;
                    }
                    //std::cout << "ok on " << port << " " << n << " " << send_region.size() << std::endl;
                    data->send_data.erase(data->send_data.begin(), data->send_data.begin() + n);
                    if (!data->send_data.empty()) {
                        return network::wait_ops::wait_write;
                    }
                } else {
                    std::cout << "close on " << port << std::endl;
                    port_map.erase(port);
                    if (!data->is_control) {
                        uint32_t msg[2];
                        msg[0] = port;
                        msg[1] = 0;
                        network::memory_region msg_region((uint8_t*)msg, 2 * 4);
                        if (port_map.contains(ctrl_port)) {
                            msg_region.push_back_into(port_map[ctrl_port]->send_data);
                            port_map[ctrl_port]->notifier.notify();
                        }
                    } else {
                        for (auto [p, m] : port_map) {
                            if (p != port) {
                                m->is_toclose = true;
                                m->notifier.notify();
                            }
                        }
                    }
                    return network::wait_ops::remove;
                }
            }

            network::memory_region recv_region;
            data->recv_buffer.resize(16 * 1024);
            recv_region.use(data->recv_buffer);
            auto [result, n] = connection.recv_data(recv_region);

            if (result) {
                if (n == 0) {
                    return network::wait_ops::wait_read;
                }
                //std::cout << "recv on " << port << " " << n << std::endl;
                data->recv_data.insert(data->recv_data.end(), data->recv_buffer.begin(), data->recv_buffer.begin() + n);
            } else {
                std::cout << "close on " << port << std::endl;
                port_map.erase(port);
                if (!data->is_control) {
                    uint32_t msg[2];
                    msg[0] = port;
                    msg[1] = 0;
                    network::memory_region msg_region((uint8_t*)msg, 2 * 4);
                    if (port_map.contains(ctrl_port)) {
                        msg_region.push_back_into(port_map[ctrl_port]->send_data);
                        port_map[ctrl_port]->notifier.notify();
                    }
                } else {
                    for (auto [p, m] : port_map) {
                        if (p != port) {
                            m->is_toclose = true;
                            m->notifier.notify();
                        }
                    }

                }
                return network::wait_ops::remove;
            }

            if (!data->recv_data.empty()) {
                //std::cout << "process on " << port << std::endl;
                if (data->is_control) {
                    while (!data->recv_data.empty()) {
                        network::memory_region msg_region;
                        msg_region.use(data->recv_data);
                        if (auto fwd_port = msg_region.at<uint32_t>(0)) {
                            if (auto fwd_size = msg_region.at<uint32_t>(1)) {
                                auto content = msg_region.splice(8, **fwd_size);
                                if (content.size() == **fwd_size) {
                                    if (port_map.contains(**fwd_port)) {
                                        content.push_back_into(port_map[**fwd_port]->send_data);
                                        port_map[**fwd_port]->notifier.notify();
                                        //std::cout << "[6580] --" << **fwd_size << "--> " << "[" << **fwd_port << "]" << std::endl;
                                        //std::cout << "forward to " << **fwd_port << std::endl;
                                    } else {
                                        //std::cout << "cannt forward to " << **fwd_port << std::endl;
                                    }
                                    data->recv_data.erase(data->recv_data.begin(), data->recv_data.begin() + **fwd_size + 8);
                                } else {
                                    return network::wait_ops::wait_read;
                                }
                            } else {
                                return network::wait_ops::wait_read;
                            }
                        } else {
                            return network::wait_ops::wait_read;
                        }
                    }

                } else {
                    uint32_t msg[2];
                    msg[0] = port;
                    msg[1] = data->recv_data.size();
                    network::memory_region msg_region((uint8_t*)msg, 2 * 4);
                    if (port_map.contains(ctrl_port)) {
                        msg_region.push_back_into(port_map[ctrl_port]->send_data);
                        network::memory_region content_region;
                        content_region.use(data->recv_data);
                        content_region.push_back_into(port_map[ctrl_port]->send_data);
                        port_map[ctrl_port]->notifier.notify();
                        //std::cout << "[" << port << "] --" << msg[1] << "--> [6580]" << std::endl;
                    }
                    data->recv_data.clear();
                }
            }

            return network::wait_ops::wait_read;
        }, 10);
    }

    if (ctrl_thread.joinable()) {
        ctrl_thread.join();
    }
    if (accept_thread.joinable()) {
        accept_thread.join();
    }


    return 0;
}
