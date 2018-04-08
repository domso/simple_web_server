#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "shared_context.h"

class http_request {
public:
    static std::pair<std::unordered_map<std::string, std::string>, std::vector<char>>
    handle_request(const std::unordered_map<std::string, std::string>& requestFields, shared_context& context);
private:    
    static void set_status(std::unordered_map<std::string, std::string>& responseFields, shared_context& context, const int status);
    static std::string filter_filename(const std::string& filename);
};
