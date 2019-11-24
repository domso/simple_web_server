#include "../web_server/web_server.h"

#include "../modules/file_loader.h"

int main(int argc, char **argv) {  
    web_server::web_server server;
    server.register_module<web_server::modules::file_loader>();
    
    web_server::config config;
    config.port = 6580;
    server.init(config);
    
    server.run();
    
    return 0;
}
