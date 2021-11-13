#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

#include "config.h"
#include "http_status_code.h"

namespace web_server {

class http_request {
public:
    http_request(config& current_config);
    std::pair<std::unordered_map<std::string, std::string>, std::vector<char>>
    handle_request(const std::unordered_map<std::string, std::string>& requestFields);
    
    template <typename T>
    void register_module(T& mod, const std::string& name = "/", const std::string& username = "", const std::string& password = "") {    
        internal_module new_module;
        new_module.name = name;
        new_module.callback = [&mod](const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string& res, const config& current_config) {
            return mod.handle(request, response, res, current_config);
        };
        new_module.authentication = {username, password};
        
        m_module_map[name] = new_module;
    }    
private:    
    std::pair<std::vector<char>, int> execute_callback(const std::unordered_map<std::string, std::string>& requestFields, std::unordered_map<std::string, std::string>& responseFields);    
    std::string get_module(const std::string& ressource);    
    
    struct internal_module {
        std::string name;
        std::pair<std::string, std::string> authentication;
        std::function<std::pair<std::vector<char>, int>(const std::unordered_map<std::string, std::string>&, std::unordered_map<std::string, std::string>&, const std::string&, const config&)> callback;
    };
    
    std::unordered_map<std::string, internal_module> m_module_map;
    
    config& m_current_config;
    http_status_code m_status_codes;
};

}
