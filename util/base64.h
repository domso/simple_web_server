#pragma once

#include <string>
#include <stdint.h>
#include <vector>

namespace util {
    namespace base64 {        
        char encode_single(const uint8_t a, const bool is_pad);
        uint8_t decode_single(const char a);
        std::string encode_tribble(const uint8_t b0, const uint8_t b1, const uint8_t b2, const int count);
        
        template<typename T>
        std::string to_base64(const T& input) {
            std::string result;
            
            char found[3];
            int count = 0;
            
            for (auto c : input) {
                found[count] = c;
                count++;
                
                if (count == 3) {
                    result += encode_tribble(found[0], found[1], found[2], count);
                    count = 0;
                }
            }   
            
            for (int i = count; i < 3; i++) {
                found[i] = 0;
            }
            
            result += encode_tribble(found[0], found[1], found[2], count);
            
            return result;
        }
        std::vector<uint8_t> base64_to_vector(const std::string& input);
    }    
}
