#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <type_traits>

#include "util/logger.h"
#include "web_server/config.h"
#include "web_server/unique_context.h"
#include "web_server/native/handle.h"

#include "status_code.h"

namespace web_server::http {       
    template<typename T>
    concept has_native_support = requires (T t, native::handle& handle) {
        t.native_callback(handle);
    };

    template<typename T>
    concept has_get_support = requires (T t, const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string& res, const config& current_config) {
        t.get_callback(request, response, res, current_config);
    };
    template<typename T>
    concept has_put_support = requires (T t, const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string& res, const config& current_config) {
        t.put_callback(request, response, res, current_config);
    };
    template<typename T>
    concept has_post_support = requires (T t, const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string& res, const config& current_config) {
        t.post_callback(request, response, res, current_config);
    };
    template<typename T>
    concept has_delete_support = requires (T t, const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string& res, const config& current_config) {
        t.delete_callback(request, response, res, current_config);
    };
    template<typename T>
    concept has_connect_support = requires (T t, const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string& res, const config& current_config) {
        t.connect_callback(request, response, res, current_config);
    };
    template<typename T>
    concept has_options_support = requires (T t, const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string& res, const config& current_config) {
        t.options_callback(request, response, res, current_config);
    };
    template<typename T>
    concept has_trace_support = requires (T t, const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string& res, const config& current_config) {
        t.trace_callback(request, response, res, current_config);
    };
    template<typename T>
    concept has_patch_support = requires (T t, const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string& res, const config& current_config) {
        t.patch_callback(request, response, res, current_config);
    };

    class requester {
    public:
        requester(config& current_config);
        std::pair<std::unordered_map<std::string, std::string>, std::vector<char>>
        handle_request(const std::string& header, const std::shared_ptr<unique_context>& context) const;
        
        template <typename T>
        void register_module(T& mod, const std::string& name = "/", const std::string& username = "", const std::string& password = "") {    
            internal_module new_module;
            new_module.name = name;

            if constexpr (has_get_support<T>) {
                new_module.get_callback = [&mod](const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string& res, const config& current_config) {
                    return mod.get_callback(request, response, res, current_config);
                };
            }
            if constexpr (has_put_support<T>) {
                new_module.put_callback = [&mod](const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string& res, const config& current_config) {
                    return mod.put_callback(request, response, res, current_config);
                };
            }
            if constexpr (has_post_support<T>) {
                new_module.post_callback = [&mod](const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string& res, const config& current_config) {
                    return mod.post_callback(request, response, res, current_config);
                };
            }
            if constexpr (has_delete_support<T>) {
                new_module.delete_callback = [&mod](const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string& res, const config& current_config) {
                    return mod.delete_callback(request, response, res, current_config);
                };
            }
            if constexpr (has_options_support<T>) {
                new_module.options_callback = [&mod](const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string& res, const config& current_config) {
                    return mod.options_callback(request, response, res, current_config);
                };
            }
            if constexpr (has_patch_support<T>) {
                new_module.patch_callback = [&mod](const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string& res, const config& current_config) {
                    return mod.patch_callback(request, response, res, current_config);
                };
            }
            if constexpr (has_connect_support<T>) {
                new_module.connect_callback = [&mod](const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string& res, const config& current_config) {
                    return mod.connect_callback(request, response, res, current_config);
                };
            }
            if constexpr (has_trace_support<T>) {
                new_module.trace_callback = [&mod](const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string& res, const config& current_config) {
                    return mod.trace_callback(request, response, res, current_config);
                };
            }

            if constexpr (has_native_support<T>) {
                new_module.native_callback = [&mod](native::handle& handle) {
                    mod.native_callback(handle);
                };
                util::logger::log_status("Loaded native module '" + std::string(T::name) + "' on '" + std::string(name) + "'");
            }

            new_module.authentication = {username, password};
            
            m_module_map[name] = new_module;
            util::logger::log_status("Loaded HTTP module '" + std::string(T::name) + "' on '" + std::string(name) + "'");
        }    
    private:    
        std::pair<std::vector<char>, int> execute_callback(const std::unordered_map<std::string, std::string>& request_fields, std::unordered_map<std::string, std::string>& response_fields, const std::shared_ptr<unique_context>& context) const;    
        std::string get_module(const std::string& ressource) const;
        std::unordered_map<std::string, std::string> insert_url_params(const std::unordered_map<std::string, std::string>& request_fields, const std::string& url) const;
            
        struct internal_module {
            std::string name;
            std::pair<std::string, std::string> authentication;
            typedef std::function<std::pair<std::vector<char>, int>(const std::unordered_map<std::string, std::string>&, std::unordered_map<std::string, std::string>&, const std::string&, const config&)> request_function;
            request_function get_callback;
            request_function put_callback;
            request_function post_callback;
            request_function delete_callback;
            request_function patch_callback;
            request_function options_callback;
            request_function trace_callback;
            request_function connect_callback;

            std::pair<std::vector<char>, int> call_by_method(const std::string& method, const std::unordered_map<std::string, std::string>&, std::unordered_map<std::string, std::string>&, const std::string&, const config&) const;

            std::function<void(native::handle&)> native_callback;
        };
        
        std::unordered_map<std::string, internal_module> m_module_map;
        
        config& m_current_config;
        status_code m_status_codes;
    };
}

