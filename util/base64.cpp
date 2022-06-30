#include "base64.h"

char util::base64::encode_single(const uint8_t a, const bool is_pad) {
    if (is_pad)  return '=';
    if (a < 26) return 65 + a;
    if (a < 52) return 97 - 26 + a;
    if (a < 62) return 48 - 52 + a;
    if (a == 62) return 43;
    if (a == 63) return 47;
    
    return 0;
}

uint8_t util::base64::decode_single(const char a) {
    if (a == '=')  return 0;
    if (a == '+')  return 62;
    if (a == '/')  return 63;
    if (a < 58) return a - 48 + 52;
    if (a < 91) return a - 65;
    if (a < 123) return a - 97 + 26;
        
    return 0;
}

std::string util::base64::encode_tribble(const uint8_t b0, const uint8_t b1, const uint8_t b2, const int count) {
    std::string result;
    
    if (count != 0) {
        uint32_t t = ((uint32_t)b0) << 16 | ((uint32_t)b1) << 8 | ((uint32_t)b2);
                
        uint8_t a[4];
        a[0] = (t & 0x00fc0000) >> 18;
        a[1] = (t & 0x0003f000) >> 12;
        a[2] = (t & 0x00000fc0) >> 6;
        a[3] = (t & 0x0000003f);
        
        
        result += encode_single(a[0], count < 0);
        result += encode_single(a[1], count < 1);
        result += encode_single(a[2], count < 2);
        result += encode_single(a[3], count < 3);
    }
    
    return result;
}


std::vector<uint8_t> util::base64::base64_to_vector(const std::string& input) {
    std::vector<uint8_t> result;
    
    char found[4];
    int count = 0;
    
    for (auto c : input) {
        found[count] = c;
        count++;
        
        if (count == 4) {            
            uint32_t t = 0;
            
            t |= (decode_single(found[0]) << 18); 
            t |= (decode_single(found[1]) << 12); 
            t |= (decode_single(found[2]) << 6); 
            t |= (decode_single(found[3]) << 0); 
            
            result.push_back((t >> 16) & 0xFF);
            
            if (found[2] != '=') {
                result.push_back((t >> 8) & 0xFF);                
            }
            
            if (found[3] != '=') {
                result.push_back((t >> 0) & 0xFF);                
            }
            
            count = 0;
        }
    }       
    
    return result;
}

