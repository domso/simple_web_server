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

int main(int argc, char **argv) {
    web_server::command_line_argument_parser cmd(argc, argv);
    network::tcp_socket<network::ipv4_addr> public_socket;
    network::tcp_socket<network::ipv4_addr> private_socket;
    network::tcp_socket<network::ipv4_addr> ctrl_socket;

    cmd.add_help_text("Proxy Sender");
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
    cmd.add_required(
        "Port of the private channel",
        [&](const std::string& port) {
            if (auto maybe_port = web_server::string_to_int<uint32_t>(port)) {
                return private_socket.accept_on(*maybe_port, 10);
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

    std::mutex mutex;
    std::condition_variable cv;
    int counter = 0;

    std::thread ctrl_thread([&]() mutable {
        while (auto ctrl_connection = ctrl_socket.accept_connection()) {
            while (true) {
                std::unique_lock ul(mutex);
                if (counter == 0) {
                    cv.wait_for(ul, std::chrono::seconds(5), [&]() {
                        return counter > 0;
                    });
                }
                while (counter > 0) {
                    **buffer_region.at<char>(0) = '1';
                    if (!ctrl_connection->send_data(buffer_region).first) {
                        break;
                    }
                    counter--;
                }

                if (counter > 0) {
                    break;
                }
                **buffer_region.at<char>(0) = '0';
                if (!ctrl_connection->send_data(buffer_region).first) {
                    break;
                }
            }
        }
    });

    while (auto public_connection = public_socket.accept_connection()) {
        {
            std::unique_lock ul(mutex);
            counter++;
            cv.notify_all();
        }

        private_socket.set_timeout(10);
        if (auto private_connection = private_socket.accept_connection()) {
            private_connection->set_timeout(0);
            std::thread t1([&, public_dc = std::move(*public_connection), private_dc = std::move(*private_connection)]() mutable {
                std::cout << "Open tunnel" << std::endl;
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

    ctrl_thread.join();

    return 0;
}
