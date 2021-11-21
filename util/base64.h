#pragma once

#include <string>
#include <stdint.h>

namespace util {
    namespace base64 {        
        char convert_single(const uint8_t a, const bool is_pad);
        std::string convert_tribble(const uint8_t b0, const uint8_t b1, const uint8_t b2, const int count);
        std::string convert_string(const std::string& input);
    }    
}
