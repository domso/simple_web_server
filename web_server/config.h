#pragma once

#include <stdint.h>
#include <string>

namespace web_server {
    struct config {        
        std::string name = "https://github.com/domso/simple_web_server";
        std::string cert = "cert.pem";
        std::string key = "key.pem";
        std::string password = "test";
        
        uint16_t port = 443;
        int max_pending = 10;
        
        std::string root_dir = "../www/";
        bool allow_file_access = true;
        
        std::string log_status = "[STATUS]";
        bool enable_log_status = true;
        std::string log_warning = "[WARNING]";
        bool enable_log_warning = true;
        std::string log_error = "[ERROR]";
        bool enable_log_error = true;
        bool enable_log_debug = true;
        
        size_t num_worker = 4;
        size_t worker_poll_timeout = 1000;
        size_t worker_poll_buffer_size = 10;
        size_t accept_poll_timeout = 1000;
        size_t accept_poll_buffer_size = 10;
    };
}
