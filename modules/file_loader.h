#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <streambuf>
#include <regex>

#include <fstream>
#include <streambuf>
#include <unordered_map>

#include "../web_server/config.h"

namespace web_server::modules {
    class file_loader {
    public:        
        std::pair<std::vector<char>, int> request_callback(const std::unordered_map<std::string, std::string>&, std::unordered_map<std::string, std::string>&, const std::string& res, const config& current_config);
    private:
        int hex_to_int(const char c);
        std::string utf8_convert(const std::string& input);
        std::string filter_filename(const std::string& filename);
        std::pair<std::vector<char>, int> load_file(const std::string& filename, const config& current_config);
    };
}

