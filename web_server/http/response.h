#pragma once

#include <unordered_map>
#include <vector>
#include <string>

namespace web_server::http {
    struct response {
        std::unordered_map<std::string, std::string> fields;
        std::vector<char> data;
        int code;
        std::string status;
    };
}
