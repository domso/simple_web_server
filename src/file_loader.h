#pragma once

#include <string>
#include <vector>

#include "config.h"

namespace web_server {
namespace file_loader {
    std::vector<char> load(const std::string& filename, const config& currentConfig);
}
}
