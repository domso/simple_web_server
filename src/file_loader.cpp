#include "file_loader.h"

#include <fstream>
#include <streambuf>

std::vector<char> file_loader::load(const std::string& filename) {
    std::vector<char> result;
    if (filename != "") {
        std::ifstream fileStream("../www" + filename, std::ios::binary);

        if (fileStream.is_open()) {
            fileStream.seekg(0, std::ios::end);
            result.reserve(fileStream.tellg());
            fileStream.seekg(0, std::ios::beg);

            result.assign((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
        }
    }

    return result;
}

