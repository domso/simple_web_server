#pragma once

#include <stdint.h>
#include <string>

namespace web_server {
    struct config {        
        std::string name = "https://github.com/domso/simple_web_server";
        std::string cert = "cert.pem";
        std::string key = "key.pem";
        
        uint16_t port = 443;
        int max_pending = 10;
        
        std::string root_dir = "../www";
        bool allow_file_access = true;
        
        std::string log_status = "[STATUS]";
        bool enable_log_status = true;
        std::string log_warning = "[WARNING]";
        bool enable_log_warning = true;
        std::string log_error = "[ERROR]";
        bool enable_log_error = true;
        
        size_t num_worker = 4;
    };
}
