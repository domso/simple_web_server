#pragma once

#include <stdint.h>
#include <string>

namespace web_server {
    struct config {
        uint16_t port = 443;
        std::string cert = "cert.pem";
        std::string key = "key.pem";
        std::string name = "https://github.com/domso/simple_web_server";
        int max_pending = 10;
        std::string root_dir = "../www";
        bool allow_file_access = true;
    };
}
