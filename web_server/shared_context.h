#pragma once

#include <functional>
#include <vector>
#include <unordered_map>
#include <memory>

#include "http_status_code.h"
#include "config.h"
#include "unique_context.h"

namespace web_server {        
    struct module_context {
        std::string name;
        std::pair<std::string, std::string> authentication;
        std::function<std::pair<std::vector<char>, int>(const std::unordered_map<std::string, std::string>&, std::unordered_map<std::string, std::string>&, const std::string&, const config&)> callback;
    };
    
    struct shared_context {
        http_status_code statusCodes;
        std::unordered_map<std::string, module_context> moduleMap;
        config currentConfig;   
    };    
}
