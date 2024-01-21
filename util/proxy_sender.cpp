#include "network/tcp_connection.h"
#include "network/tcp_socket.h"
#include "network/socket_pipe.h"
#include "network/socket_container.h"
#include "sys/epoll.h"
#include "signal.h"

#include "signal.h"
#include "command_line_argument_parser.h"
#include "string_to_int.h"

#include <thread>
#include <mutex>
#include <iostream>

int main(int argc, char **argv) {
    web_server::command_line_argument_parser cmd(argc, argv);
    network::ipv4_addr public_addr;
    network::ipv4_addr private_addr;
    network::ipv4_addr ctrl_addr;

    cmd.add_help_text("Proxy Sender");
    cmd.add_required(
        "IP and port of the control channel",
        [&](const std::string& ip, const std::string& port) {
            if (auto maybe_port = web_server::string_to_int<uint32_t>(port)) {
                return ctrl_addr.init(ip, *maybe_port);
            }   
            return false;
        },  
        "--control"
    );  
    cmd.add_required(
        "IP and port of the public channel",
        [&](const std::string& ip, const std::string& port) {
            if (auto maybe_port = web_server::string_to_int<uint32_t>(port)) {
                return public_addr.init(ip, *maybe_port);
            }   
            return false;
        },  
        "--public"
    );  
    cmd.add_required(
        "IP and port of the private channel",
        [&](const std::string& ip, const std::string& port) {
            if (auto maybe_port = web_server::string_to_int<uint32_t>(port)) {
                return private_addr.init(ip, *maybe_port);
            }   
            return false;
        },  
        "--private"
    );  

    if (!cmd.parse_arguments()) {
        return -1;
    }

    signal(SIGPIPE, SIG_IGN);

    std::vector<int> buffer(1);
    network::memory_region buffer_region;
    buffer_region.use(buffer);

    while (true) {
        network::tcp_connection<network::ipv4_addr> ctrl_connection;
        ctrl_connection.set_timeout(6);

        if (ctrl_connection.connect_to(ctrl_addr)) {
            ctrl_connection.set_timeout(0);
            while (true) {
                **buffer_region.at<char>(0) = '0';
                while (**buffer_region.at<char>(0) == '0' && ctrl_connection.recv_data(buffer_region).first) {}
                if (**buffer_region.at<char>(0) == '0') {
                    break;
                }

                network::tcp_connection<network::ipv4_addr> private_connection;
                network::tcp_connection<network::ipv4_addr> public_connection;

                private_connection.set_timeout(10);
                if (private_connection.connect_to(private_addr)) {
                    private_connection.set_timeout(0);

                    public_connection.set_timeout(120);
                    if (public_connection.connect_to(public_addr)) {
                        public_connection.set_timeout(0);
                        std::thread t1([&, public_dc = std::move(public_connection), private_dc = std::move(private_connection)]() mutable {
                            std::cout << "open tunnel" << std::endl;

                            network::socket_container<network::tcp_connection<network::ipv4_addr>&, void*> sockets(10);

                            void* p = nullptr;

                            network::socket_pipe input_pipe;
                            network::socket_pipe output_pipe;

                            sockets.add_socket(public_dc, p, false);
                            sockets.add_socket(private_dc, p, false);
                            bool running = true;

                            while (running) {
                                sockets.wait([&](network::tcp_connection<network::ipv4_addr>& connection, void*, network::socket_container_notifier&) {
                                    if (connection.get_socket() == public_dc.get_socket()) {
                                        if (!input_pipe.forward_from_to(connection, private_dc)) {
                                            running = false;
                                        }

                                        return network::wait_ops::wait_read;
                                    } else {
                                        if (!output_pipe.forward_from_to(connection, public_dc)) {
                                            running = false;
                                        }

                                        return network::wait_ops::wait_read;
                                    }
                                }, 10);
                            }

                            std::cout << "close tunnel" << std::endl;
                        });
                        t1.detach();
                    }
                }
            }
        }
    }

    return 0;
}
