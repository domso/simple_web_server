#include <thread>
#include "network/tcp_socket.h"
#include "network/tcp_connection.h"
#include "network/ssl_context.h"
#include "network/ssl_connection.h"
#include "connection_thread.h"
#include "shared_context.h"


int main(int argc, char **argv) {    
    network::tcp_socket<network::ipv4_addr> socket;
    network::ssl_context<network::ipv4_addr> sslContext;   
    shared_context context;
    socket.accept_on(6580, 10);  
    if (!sslContext.init("cert.pem", "key.pem")) {
        std::cout << "Could not load SSL" << std::endl;
        return 0;
    }
    
    while (true) {
        auto tcpConnection = socket.accept_connection();    
        if (!tcpConnection) {
            // skt is broken
            return 0;
        }     
        
        auto sslConnection = sslContext.accept(std::move(*tcpConnection));
        if (sslConnection) {
            std::thread newThread(&connection_thread::main, std::move(*sslConnection), std::ref(context));
            newThread.detach();
        }       
        
    }
    
    return 0;
}
