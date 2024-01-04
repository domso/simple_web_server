#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <streambuf>
#include <regex>

#include <fstream>
#include <streambuf>
#include <unordered_map>

#include "web_server/http/request.h"
#include "web_server/http/response.h"

namespace web_server::modules {
    class file_loader {
    public:        
        typedef int userdata;
        http::response get_callback(const http::request& request, userdata& local);
    private:
        int hex_to_int(const char c);
        std::string utf8_convert(const std::string& input);
        std::string filter_filename(const std::string& filename);
        http::response load_file(const std::string& filename);
    };
}

