#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <type_traits>

#include "web_server/config.h"
#include "web_server/unique_context.h"
#include "web_server/native/handle.h"

#include "status_code.h"

namespace web_server::http {       
    template<typename T>
    concept has_native_support = requires (T t) {
        t.native_callback;
    };
        
    class requester {
    public:
        requester(config& current_config);
        std::pair<std::unordered_map<std::string, std::string>, std::vector<char>>
        handle_request(const std::unordered_map<std::string, std::string>& request_fields, const std::shared_ptr<unique_context>& context) const;
        
        template <typename T>
        void register_module(T& mod, const std::string& name = "/", const std::string& username = "", const std::string& password = "") {    
            internal_module new_module;
            new_module.name = name;
            new_module.request_callback = [&mod](const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string& res, const config& current_config) {
                return mod.request_callback(request, response, res, current_config);
            };
            new_module.authentication = {username, password};
            
            m_module_map[name] = new_module;
        }    
            
        template <has_native_support T>
        void register_module(T& mod, const std::string& name = "/", const std::string& username = "", const std::string& password = "") {    
            internal_module new_module;
            new_module.name = name;
            new_module.request_callback = [&mod](const std::unordered_map<std::string, std::string>& request, std::unordered_map<std::string, std::string>& response, const std::string& res, const config& current_config) {
                return mod.request_callback(request, response, res, current_config);
            };
            new_module.native_callback = [&mod](native::handle& handle) {
                mod.native_callback(handle);
            };
                    
            new_module.authentication = {username, password};
            
            m_module_map[name] = new_module;
        }   
    private:    
        std::pair<std::vector<char>, int> execute_callback(const std::unordered_map<std::string, std::string>& request_fields, std::unordered_map<std::string, std::string>& response_fields, const std::shared_ptr<unique_context>& context) const;    
        std::string get_module(const std::string& ressource) const;
            
        struct internal_module {
            std::string name;
            std::pair<std::string, std::string> authentication;
            std::function<std::pair<std::vector<char>, int>(const std::unordered_map<std::string, std::string>&, std::unordered_map<std::string, std::string>&, const std::string&, const config&)> request_callback;
            std::function<void(native::handle&)> native_callback;
        };
        
        std::unordered_map<std::string, internal_module> m_module_map;
        
        config& m_current_config;
        status_code m_status_codes;
    };
}

