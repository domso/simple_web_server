#include "accept.h"

#include "openssl/sha.h"
#include "util/base64.h"

std::pair<std::vector<char>, int> web_server::modules::web_socket::accept::request_callback(const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string&, const config&) const {
    if (request.count("Sec-WebSocket-Key") > 0) {
        if (request.count("Upgrade") > 0) {
            response["Upgrade"] = trim_string(request.at("Upgrade"));
        }
        if (request.count("Connection") > 0) {
            response["Connection"] += ", " + trim_string(request.at("Connection"));
        }
        
        response["Sec-WebSocket-Accept"] = websocket_response_key(trim_string(request.at("Sec-WebSocket-Key")));
        
        return {{}, 101};
    }
    
    return {{}, 404};
}

void web_server::modules::web_socket::accept::native_callback(native::handle& handle) const {
    if (on_new_socket) {
        web_socket new_skt(handle);
        on_new_socket(new_skt);
    }                    
}

std::string web_server::modules::web_socket::accept::trim_string(const std::string& s) const {        
    if (s.length() > 3) {
        return s.substr(1, s.length() - 3);
    } else {
        return "";
    }
}

std::string web_server::modules::web_socket::accept::websocket_response_key(const std::string& key) const {
    std::string guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    std::string combined = key + guid;
    uint8_t key_buffer[20];
    SHA1(reinterpret_cast<const uint8_t*>(combined.c_str()), combined.length(), key_buffer);
    return util::base64::to_base64(std::string(reinterpret_cast<const char*>(key_buffer), 20));
}   





