#include "../web_server/web_server.h"
#include "../modules/file_loader.h"
#include "../modules/web_socket/web_socket.h"

int main(int argc, char **argv) {    
    web_server::modules::file_loader file_loader;
    web_server::modules::web_socket web_socket;    
    
    web_server::web_server server;
    server.register_module(file_loader, "/");
    server.register_module(web_socket, "/ws");
    
    web_server::config config;
    config.port = 6580;
    server.init(config);
    
    server.run();
    
    return 0;
}
