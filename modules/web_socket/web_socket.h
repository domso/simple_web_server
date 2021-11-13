#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "../../web_server/config.h"
#include "../../util/base64.h"

#include "openssl/sha.h"

namespace web_server {
    namespace modules {
        class web_socket {
        public:
            std::pair<std::vector<char>, int> handle(const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string&, const config&) const {
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
        private:
            std::string trim_string(const std::string& s) const {        
                if (s.length() > 3) {
                    return s.substr(1, s.length() - 3);
                } else {
                    return "";
                }
            }
            
            std::string websocket_response_key(const std::string& key) const {
                std::string guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
                std::string combined = key + guid;
                uint8_t key_buffer[20];
                SHA1(reinterpret_cast<const uint8_t*>(combined.c_str()), combined.length(), key_buffer);
                return util::base64::convert_string(std::string(reinterpret_cast<const char*>(key_buffer), 20));
            }            
        };
    }
}


//                 auto read_region = context.recv_buffer.readable_region();
                                
//                 auto n = context.frame_decoder.unpack_data(read_region, [&](const network::memory_region region, const uint8_t header) {                    
//                     region.push_back_into(context.recv_data);
//                     
//                     if ((header & 128) > 0) {                                         
//                         std::string response = "hello world";
//                         network::memory_region response_region;
//                         response_region.use(response);
//                         context.frame_encoder.pack_data(response_region, [&](const network::memory_region region) {
//                             region.push_back_into(context.response_data);
//                         });
//                                                 
//                         context.recv_data.clear();
//                     }                    
//                 });
//                 
//                 context.recv_buffer.read(read_region.splice(0, n));

                
     
//     } else {    
    
    
    
    



    
