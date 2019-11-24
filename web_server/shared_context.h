#pragma once

#include <functional>
#include <vector>
#include <unordered_map>

#include "http_status_code.h"
#include "config.h"

namespace web_server {    
    struct module_context {
        std::string name;
        std::pair<std::string, std::string> authentication;
        std::function<std::pair<std::vector<char>, int>(const std::string&, const config&)> callback;
    };
    
    struct shared_context {
        http_status_code statusCodes;
        std::unordered_map<std::string, module_context> moduleMap;
        config currentConfig;   
    };
}
