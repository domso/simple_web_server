#pragma once

#include <string>
#include <vector>

namespace file_loader {
    std::vector<char> load(const std::string& filename);
}
