#include "file_loader.h"

#include <fstream>
#include <streambuf>

std::vector<char> web_server::file_loader::load(const std::string& filename, const config& currentConfig) {
    std::vector<char> result;
    if (filename != "") {
        std::ifstream fileStream(currentConfig.root_dir + filename, std::ios::binary);

        if (fileStream.is_open()) {
            fileStream.seekg(0, std::ios::end);
            result.reserve(fileStream.tellg());
            fileStream.seekg(0, std::ios::beg);

            result.assign((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
        }
    }

    return result;
}

