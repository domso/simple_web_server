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
    static std::pair<std::vector<char>, int> execute_callback(const std::unordered_map<std::string, std::string>& requestFields, const shared_context& context);    
    static std::string filter_filename(const std::string& filename);
};

}
