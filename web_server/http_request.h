#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include "shared_context.h"

namespace web_server {

class http_request {
public:
    static std::pair<std::unordered_map<std::string, std::string>, std::vector<char>>
    handle_request(const std::unordered_map<std::string, std::string>& requestFields, shared_context& context);
private:    
    static std::pair<std::vector<char>, int> execute_callback(const std::unordered_map<std::string, std::string>& requestFields, std::unordered_map<std::string, std::string>& responseFields, const shared_context& context);    
    static std::string get_module(const std::string& ressource);
    
    static char single_to_base64(const uint8_t a, const bool isPad);
    static std::string tripple_to_base64(const char b0, const char b1, const char b2, const int count);
    static std::string string_to_base64(const std::string& input);
};

}
