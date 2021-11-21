#include "web_server/web_server.h"
#include "modules/file_loader.h"
#include "modules/web_socket/accept.h"

#include "util/logger.h"

int main(int argc, char **argv) {   
    web_server::modules::file_loader file_loader;
    web_server::modules::web_socket::accept web_socket_accept;
    
    web_socket_accept.on_new_socket = [](web_server::modules::web_socket::web_socket& socket) {        
        socket.set_on_recv([s = web_server::modules::web_socket::web_socket(socket)](const network::memory_region region) mutable {      
            s.send(region);
        });
        socket.set_on_close([](){
            util::logger::log_debug("Close connection");
        });
    };    
    
    web_server::web_server server;
    server.register_module(file_loader, "/");
    server.register_module(web_socket_accept, "/ws");
    
    web_server::config config;
    config.port = 6580;
    util::logger::init(config);
    
    server.init(config);
    
    server.run();
    
    return 0;
}
