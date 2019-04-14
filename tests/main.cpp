#include "../src/web_server.h"

class test_request {
public:
    typedef int handle_arg_t;
    constexpr static auto name = "/test";
    static std::pair<std::vector<char>, int> handle(handle_arg_t* arg) {
        
        std::string text = "<html>test</html>";
        
        
        return {std::vector<char>(text.begin(), text.end()), 200};
    }
private:
};

int main(int argc, char **argv) {    
    web_server::web_server server;
    
    server.register_new_handle<test_request>();
    
    web_server::config config;
    config.port = 6580;
    server.init(config);
    server.run();
    
    return 0;
}
