#pragma once

#include <string>
#include <vector>
#include <memory>

#include "web_socket.h"

#include "openssl/sha.h"
#include "util/base64.h"

namespace web_server::modules::web_socket {
    template<typename T>
    class accept {
    public:
        struct userdata {
            web_socket socket;
            typename T::private_context private_context;
        };

        http::response get_callback(const http::request& request, userdata& local) {
            http::response response;
            response.code = 404;

            if (request.fields.count("Sec-WebSocket-Key") > 0) {
                if (request.fields.count("Upgrade") > 0) {
                    response.fields["Upgrade"] = trim_string(request.fields.at("Upgrade"));
                }
                if (request.fields.count("Connection") > 0) {
                    response.fields["Connection"] += ", " + trim_string(request.fields.at("Connection"));
                }
                
                response.fields["Sec-WebSocket-Accept"] = websocket_response_key(trim_string(request.fields.at("Sec-WebSocket-Key")));
                
                response.code = 101;
            }
            
            return response;
        }

        size_t native_recv_callback(network::memory_region read_region, userdata& local) {
            return local.socket.recv(read_region, [&](network::memory_region_view data, web_socket& socket) {
                T::on_recv(data, socket, local.private_context, shared_context);
            });
        }
        void native_open_callback(userdata& local, network::socket_container_notifier& notifier) {
            local.socket.init(notifier);
            T::on_init(local.socket, local.private_context, shared_context);
        }
        bool native_status_callback(userdata& local) {
            return local.socket.status();
        }
        void native_update_callback(std::vector<char>& buffer, userdata& local) {
            local.socket.update(buffer);
        }

        typename T::shared_context shared_context;
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
            return util::base64::to_base64(std::string(reinterpret_cast<const char*>(key_buffer), 20));
        }
    };
}
