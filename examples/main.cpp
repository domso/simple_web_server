#include "web_server/web_server.h"
#include "modules/file_loader.h"
#include "modules/web_socket/accept.h"

int main(int argc, char **argv) {         
    web_server::config config;
    config.port = 6580;
    util::logger::init(config);

    web_server::web_server server;

    if (server.init(config)) {    
         server.run();
    }
    
    return 0;
}
