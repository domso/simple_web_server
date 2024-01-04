#pragma once

#include <string>
#include <unordered_map>

namespace web_server::http {
    struct request {
        std::string module;
        std::string method;
        std::unordered_map<std::string, std::string> fields;
        std::unordered_map<std::string, std::string> parameter;
        std::string resource;
    };
}
