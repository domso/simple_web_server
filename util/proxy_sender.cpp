#include "network/tcp_connection.h"
#include "network/tcp_socket.h"
#include "network/os_pipe.h"
#include "signal.h"

#include "signal.h"

#include <thread>
#include <mutex>
#include <iostream>

int main() {
    network::ipv4_addr public_addr;
    network::ipv4_addr private_addr;

    if (!public_addr.init("127.0.0.1", 6580)) {
        return -1;
    }
    if (!private_addr.init("127.0.0.1", 6590)) {
        return -1;
    }

    signal(SIGPIPE, SIG_IGN);

    std::vector<int> buffer(5);
    network::memory_region buffer_region;
    buffer_region.use(buffer);


    while (true) {
        network::tcp_connection<network::ipv4_addr> private_connection;
        network::tcp_connection<network::ipv4_addr> public_connection;
        private_connection.set_timeout(6);

        if (private_connection.connect_to(private_addr)) {
            private_connection.set_timeout(0);
            std::cout << "has now connection to recv" << std::endl;
            if (!private_connection.recv_data(buffer_region).first) {
                continue;
            }
            std::cout << "now its going to start" << std::endl;

            if (public_connection.connect_to(public_addr)) {
                std::thread t1([&, public_dc = std::move(public_connection), private_dc = std::move(private_connection)]() mutable {
                    std::cout << "open tunnel" << std::endl;
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
    }

    return 0;
}


