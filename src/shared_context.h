#pragma once

#include "http_status_code.h"
#include <functional>
#include <vector>
#include <unordered_map>
#include "config.h"

namespace web_server {

struct shared_context {    
    http_status_code statusCodes; 
    std::unordered_map<std::string, std::function<std::pair<std::vector<char>, int>(void*)>> callbackMap;
    config currentConfig;
};

}
