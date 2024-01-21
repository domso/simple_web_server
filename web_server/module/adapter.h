#pragma once

#include <string>
#include <vector>
#include <memory>
#include <type_traits>

#include "util/logger.h"
#include "util/base64.h"
#include "web_server/unique_context.h"
#include "web_server/http/parser.h"
#include "web_server/http/status_code.h"

#include "util/pack_instances.h"
#include "util/pack_iteratable.h"
#include "util/named_type.h"

#include "invoker.h"

namespace web_server::module {       
    template<typename T_config, typename T, typename... Ts>
    class adapter {
    public:
        template<typename Treq>
        auto& get_module() {
            return m_mod_invoker.template get<Treq>();
        }
        template<typename Treq>
        const auto& get_module() const {
            return m_mod_invoker.template get<Treq>();
        }

        size_t handle_http_request(const network::memory_region read_region, std::shared_ptr<unique_context<T, Ts...>>& context, network::socket_container_notifier& notifier) {
            for (size_t i = 3; i < read_region.size(); i++) {
                if (read_region.data()[i - 0] == '\n' &&
                    read_region.data()[i - 1] == '\r' &&
                    read_region.data()[i - 2] == '\n' &&
                    read_region.data()[i - 3] == '\r'                
                ) {                         
                    std::string header = read_region.splice(0, i + 1).template export_to<std::string>();
                    
                    http::response response;
                    if (auto request = http::parser::parse_request(header)) {
                        util::logger::log_debug("Requested ressource " + request->resource);
                        util::logger::log_debug("Requested module " + request->module);

                        response = m_mod_invoker.call(*request, context, notifier);

                        if (response.code == 0) {
                            response.code = 404;
                        }

                        response.fields["Content-Length"] = std::to_string(response.data.size());
                        response.status = m_status_codes.get(response.code);
                    } else {
                        response.status = m_status_codes.get(400);
                    }

                    response.fields["Server"] = std::string(T_config::template get_value<"name">().data());
                    response.fields["Connection"] += ", keep-alive";

                    
                    auto response_header = build_response_header(response);
                    context->response_data.insert(context->response_data.end(), response_header.begin(), response_header.end());
                    context->response_data.insert(context->response_data.end(), response.data.begin(), response.data.end());

                    return i + 1;
                }
            }

            return 0;
        }
        size_t handle_native_request(const int selected_module, const network::memory_region read_region, std::shared_ptr<unique_context<T, Ts...>>& context) {
            return m_mod_invoker.call_recv_by_number(selected_module, read_region, context);
        }
        void handle_native_update(const int selected_module, std::vector<char>& buffer, std::shared_ptr<unique_context<T, Ts...>>& context) {
            m_mod_invoker.call_update_by_number(selected_module, buffer, context);
        }
        bool handle_native_status(const int selected_module, std::shared_ptr<unique_context<T, Ts...>>& context) {
            return m_mod_invoker.call_status_by_number(selected_module, context);
        }
    private:    
        std::string build_response_header(const http::response& response) const {
            std::string result = "HTTP/1.1 " + std::to_string(response.code) + " " + response.status + "\r\n";   
            
            for (auto& key_value : response.fields) {
                result += key_value.first + ": " + key_value.second + "\r\n";
            }
            
            result += "\r\n";
            
            return result;
        }

        invoker<T, Ts...> m_mod_invoker;
        http::status_code m_status_codes;
    };
}
