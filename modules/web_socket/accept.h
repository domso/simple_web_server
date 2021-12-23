#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "web_server/config.h"
#include "web_server/native/handle.h"
#include "web_socket.h"

namespace web_server::modules::web_socket {
    class accept {
    public:
        std::pair<std::vector<char>, int> request_callback(const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string&, const config&) const;                
        void native_callback(native::handle& handle) const;
        
        std::function<void(web_socket& skt)> on_new_socket;
    private:
        std::string trim_string(const std::string& s) const;                
        std::string websocket_response_key(const std::string& key) const;               
    };
}




    
