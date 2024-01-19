
#include "network/tcp_connection.h"
#include "network/tcp_socket.h"
#include "network/os_pipe.h"
#include "signal.h"

#include <thread>
#include <mutex>
#include <iostream>
#include <condition_variable>

int main() {
    network::tcp_socket<network::ipv4_addr> public_data_socket;
    network::tcp_socket<network::ipv4_addr> private_data_socket;

    if (!public_data_socket.accept_on(6585, 10)) {
        return -1;
    }
    if (!private_data_socket.accept_on(6590, 10)) {
        return -1;
    }

    signal(SIGPIPE, SIG_IGN);

    std::vector<int> buffer(5);
    network::memory_region buffer_region;
    buffer_region.use(buffer);

    while (auto private_data_connection = private_data_socket.accept_connection()) {
        if (auto public_data_connection = public_data_socket.accept_connection()) {
            if (!private_data_connection->send_data(buffer_region).first) {
                continue;
            }
            std::thread t1([&, public_dc = std::move(*public_data_connection), private_dc = std::move(*private_data_connection)]() mutable {
                std::cout << "Open tunnel" << std::endl;
                std::thread t2([&]() mutable {
                    network::os_pipe p;
                    if (p.is_valid()) {
                        while (true) {
                            if (!p.read_from(public_dc)) {
                                return;
                            }
                            if (!p.write_to(private_dc)) {
                                return;
                            }
                        }
                    }
                });
                std::thread t3([&]() mutable {
                    network::os_pipe p;
                    if (p.is_valid()) {
                        while (true) {
                            if (!p.read_from(private_dc)) {
                                return;
                            }
                            if (!p.write_to(public_dc)) {
                                return;
                            }
                        }
                    }
                });
                
                if (t2.joinable()) {
                    t2.join();
                }
                if (t3.joinable()) {
                    t3.join();
                }
                std::cout << "close tunnel" << std::endl;
            });
            t1.detach();
        }
    }


    return 0;
}
