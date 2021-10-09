#include "http_request.h"
// #include "file_loader.h"

#include <openssl/sha.h>
#include <iostream>

std::pair<std::unordered_map<std::string, std::string>, std::vector<char>> 
web_server::http_request::handle_request(const std::unordered_map<std::string, std::string>& requestFields, shared_context& context) {
    std::unordered_map<std::string, std::string> responseFields;
    
    responseFields["Server"] = context.currentConfig.name;
    responseFields["Connection"] = "keep-alive";
    
    std::pair<std::vector<char>, int> output = execute_callback(requestFields, responseFields, context);

    responseFields["Content-Length"] = std::to_string(output.first.size());
    responseFields["STATUS"] = std::to_string(output.second) + " " + context.statusCodes.get(output.second);
        
    return std::make_pair(responseFields, output.first);
}

std::pair<std::vector<char>, int> web_server::http_request::execute_callback(const std::unordered_map<std::string, std::string>& requestFields, std::unordered_map<std::string, std::string>& responseFields, const shared_context& context) {
    std::string requestedResource = requestFields.at("GET").substr(0, requestFields.at("GET").find_first_of("?#"));
    std::string requestedModule = get_module(requestedResource);    
    std::pair<std::vector<char>, int> result;
    
    result.second = 404;    
    
    if (requestFields.count("Sec-WebSocket-Key") > 0) {
        if (requestFields.count("Upgrade") > 0) {
            responseFields["Upgrade"] = trim_string(requestFields.at("Upgrade"));
        }
        if (requestFields.count("Connection") > 0) {
            responseFields["Connection"] += ", " + trim_string(requestFields.at("Connection"));
        }
        
        responseFields["Sec-WebSocket-Accept"] = websocket_response_key(trim_string(requestFields.at("Sec-WebSocket-Key")));
        
        result.second = 101;

    } else {    
        auto search = context.moduleMap.find(requestedModule);    
        if (search != context.moduleMap.end()) {
            const module_context& currentModule = search->second;
                
            if (currentModule.authentication.first == "") {
                result = search->second.callback(requestedResource, context.currentConfig);            
            } else {
                if (requestFields.count("Authorization") == 0 || requestFields.at("Authorization") != " Basic " + string_to_base64(currentModule.authentication.first + ":" + currentModule.authentication.second) + "\r\n") {                
                    responseFields["WWW-Authenticate"] = "Basic realm=\"" + search->second.name + "\"";
                    result.second = 401;
                } else {    
                    result = search->second.callback(requestedResource, context.currentConfig);                            
                }
            }
        }
    }
    
    return result;
}

std::string web_server::http_request::get_module(const std::string& ressource) {
    auto pos = ressource.find_first_of("/", 1);
    
    if (pos == std::string::npos) {
        return "/";
    } else {
        return ressource.substr(0, pos);
    }    
}


std::string web_server::http_request::trim_string(const std::string& s) {        
    if (s.length() > 3) {
        return s.substr(1, s.length() - 3);
    } else {
        return "";
    }
}

char web_server::http_request::single_to_base64(const uint8_t a, const bool isPad) {
    if (isPad)  return '=';
    if (a < 26) return 65 + a;
    if (a < 52) return 97 - 26 + a;
    if (a < 62) return 48 - 52 + a;
    if (a == 62) return 43;
    if (a == 63) return 47;
    
    return 0;
}

std::string web_server::http_request::tripple_to_base64(const uint8_t b0, const uint8_t b1, const uint8_t b2, const int count) {
    std::string result;
    
    if (count != 0) {
        uint32_t t = ((uint32_t)b0) << 16 | ((uint32_t)b1) << 8 | ((uint32_t)b2);
                
        uint8_t a[4];
        a[0] = (t & 0x00fc0000) >> 18;
        a[1] = (t & 0x0003f000) >> 12;
        a[2] = (t & 0x00000fc0) >> 6;
        a[3] = (t & 0x0000003f);
        
        
        result += single_to_base64(a[0], count < 0);
        result += single_to_base64(a[1], count < 1);
        result += single_to_base64(a[2], count < 2);
        result += single_to_base64(a[3], count < 3);
    }
    
    return result;
}

std::string web_server::http_request::string_to_base64(const std::string& input) {
    std::string result;
    
    char found[3];
    int count = 0;
    
    for (auto c : input) {
        found[count] = c;
        count++;
        
        if (count == 3) {
            result += tripple_to_base64(found[0], found[1], found[2], count);
            count = 0;
        }
    }   
    
    for (int i = count; i < 3; i++) {
        found[i] = 0;
    }
    
    result += tripple_to_base64(found[0], found[1], found[2], count);
    
    return result;
}

std::string web_server::http_request::websocket_response_key(const std::string& key) {
    std::string guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    std::string combined = key + guid;
    uint8_t key_buffer[20];
    SHA1(reinterpret_cast<const uint8_t*>(combined.c_str()), combined.length(), key_buffer);
    return string_to_base64(std::string(reinterpret_cast<const char*>(key_buffer), 20));
}

