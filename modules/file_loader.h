#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <streambuf>
#include <regex>

#include <fstream>
#include <streambuf>

#include "../web_server/config.h"

namespace web_server {
    namespace modules {
        class file_loader {
        public:        
            static bool init();    
            static std::pair<std::vector<char>, int> handle(const std::string& res, const config& current_config);
        private:
            static int hex_to_int(char c);
            static std::string utf8_convert(const std::string& input);
            static std::string filter_filename(const std::string& filename);
            static std::pair<std::vector<char>, int> load_file(const std::string& filename, const config& currentConfig);
        };
    }
}
