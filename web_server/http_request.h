#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

#include "shared_context.h"

namespace web_server {

class http_request {
public:
    http_request(shared_context& context);
    std::pair<std::unordered_map<std::string, std::string>, std::vector<char>>
    handle_request(const std::unordered_map<std::string, std::string>& requestFields);
private:    
    std::pair<std::vector<char>, int> execute_callback(const std::unordered_map<std::string, std::string>& requestFields, std::unordered_map<std::string, std::string>& responseFields);    
    std::string get_module(const std::string& ressource);
    
    shared_context& m_context;
};

}
