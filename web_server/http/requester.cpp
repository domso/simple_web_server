#include "requester.h"

#include "util/base64.h"

web_server::http::requester::requester(config& current_config) : m_current_config(current_config) {}

std::pair<std::unordered_map<std::string, std::string>, std::vector<char>> 
web_server::http::requester::handle_request(const std::unordered_map<std::string, std::string>& request_fields, const std::shared_ptr<unique_context>& context) const {
    std::unordered_map<std::string, std::string> response_fields;
    
    response_fields["Server"] = m_current_config.name;
    response_fields["Connection"] = "keep-alive";
    
    std::pair<std::vector<char>, int> output = execute_callback(request_fields, response_fields, context);

    response_fields["Content-Length"] = std::to_string(output.first.size());
    response_fields["STATUS"] = std::to_string(output.second) + " " + m_status_codes.get(output.second);
        
    return std::make_pair(response_fields, output.first);
}

std::pair<std::vector<char>, int> web_server::http::requester::execute_callback(const std::unordered_map<std::string, std::string>& request_fields, std::unordered_map<std::string, std::string>& response_fields, const std::shared_ptr<unique_context>& context) const {
    std::string requested_resource = request_fields.at("GET").substr(0, request_fields.at("GET").find_first_of("?#"));
    std::string requested_module = get_module(requested_resource);    
    std::pair<std::vector<char>, int> result;
    
    result.second = 404;
    auto search = m_module_map.find(requested_module);    
    if (search == m_module_map.end()) {
        search = m_module_map.find("/");          
    }
        
    if (search != m_module_map.end()) {
        const internal_module& current_module = search->second;
            
        if (current_module.authentication.first == "") {
            result = current_module.request_callback(request_fields, response_fields, requested_resource, m_current_config);
        } else {
            if (request_fields.count("Authorization") == 0 || request_fields.at("Authorization") != " Basic " + util::base64::to_base64(current_module.authentication.first + ":" + current_module.authentication.second) + "\r\n") {                
                response_fields["WWW-Authenticate"] = "Basic realm=\"" + current_module.name + "\"";
                result.second = 401;
            } else {    
                result = current_module.request_callback(request_fields, response_fields, requested_resource, m_current_config);                            
            }
        }        
            
        if (result.second == 101) {
            native::handle handle(context);
            if (current_module.native_callback) {
                current_module.native_callback(handle);
                context->is_native = true;                
            } else {
                result.second = 404;
            }
        }
    }
    
    return result;
}

std::string web_server::http::requester::get_module(const std::string& ressource) const {
    auto pos = ressource.find_first_of("/", 1);
    
    if (pos == std::string::npos) {
        return "/";
    } else {
        return ressource.substr(0, pos);
    }    
}



