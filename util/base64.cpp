#include "base64.h"

char util::base64::convert_single(const uint8_t a, const bool is_pad) {
    if (is_pad)  return '=';
    if (a < 26) return 65 + a;
    if (a < 52) return 97 - 26 + a;
    if (a < 62) return 48 - 52 + a;
    if (a == 62) return 43;
    if (a == 63) return 47;
    
    return 0;
}

std::string util::base64::convert_tribble(const uint8_t b0, const uint8_t b1, const uint8_t b2, const int count) {
    std::string result;
    
    if (count != 0) {
        uint32_t t = ((uint32_t)b0) << 16 | ((uint32_t)b1) << 8 | ((uint32_t)b2);
                
        uint8_t a[4];
        a[0] = (t & 0x00fc0000) >> 18;
        a[1] = (t & 0x0003f000) >> 12;
        a[2] = (t & 0x00000fc0) >> 6;
        a[3] = (t & 0x0000003f);
        
        
        result += convert_single(a[0], count < 0);
        result += convert_single(a[1], count < 1);
        result += convert_single(a[2], count < 2);
        result += convert_single(a[3], count < 3);
    }
    
    return result;
}

std::string util::base64::convert_string(const std::string& input) {
    std::string result;
    
    char found[3];
    int count = 0;
    
    for (auto c : input) {
        found[count] = c;
        count++;
        
        if (count == 3) {
            result += convert_tribble(found[0], found[1], found[2], count);
            count = 0;
        }
    }   
    
    for (int i = count; i < 3; i++) {
        found[i] = 0;
    }
    
    result += convert_tribble(found[0], found[1], found[2], count);
    
    return result;
}

