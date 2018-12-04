#include <thread>
#include "network/tcp_socket.h"
#include "network/tcp_connection.h"
#include "connection_thread.h"
#include "shared_context.h"

int main(int argc, char **argv) {    
    network::tcp_socket<network::ipv4_addr> socket;    
    shared_context context;
    socket.accept_on(6580, 10);    
    
    while (true) {
        auto newConnection = socket.accept_connection();
    
        if (!newConnection) {
            // skt is broken
            return 0;
        }        
        
        std::thread newThread(&connection_thread::main, std::move(*newConnection), std::ref(context));
        newThread.detach();
    }
    
    return 0;
}
