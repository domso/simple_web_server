#include "http_request.h"
#include <iostream>
#include "../util/base64.h"

web_server::http_request::http_request(config& current_config) : m_current_config(current_config) {}

std::pair<std::unordered_map<std::string, std::string>, std::vector<char>> 
web_server::http_request::handle_request(const std::unordered_map<std::string, std::string>& requestFields) {
    std::unordered_map<std::string, std::string> responseFields;
    
    responseFields["Server"] = m_current_config.name;
    responseFields["Connection"] = "keep-alive";
    
    std::pair<std::vector<char>, int> output = execute_callback(requestFields, responseFields);

    responseFields["Content-Length"] = std::to_string(output.first.size());
    responseFields["STATUS"] = std::to_string(output.second) + " " + m_status_codes.get(output.second);
        
    return std::make_pair(responseFields, output.first);
}

std::pair<std::vector<char>, int> web_server::http_request::execute_callback(const std::unordered_map<std::string, std::string>& requestFields, std::unordered_map<std::string, std::string>& responseFields) {
    std::string requestedResource = requestFields.at("GET").substr(0, requestFields.at("GET").find_first_of("?#"));
    std::string requestedModule = get_module(requestedResource);    
    std::pair<std::vector<char>, int> result;
    
    result.second = 404;

    auto search = m_module_map.find(requestedModule);    
        
    if (search != m_module_map.end()) {
        const internal_module& currentModule = search->second;
            
        if (currentModule.authentication.first == "") {
            result = search->second.callback(requestFields, responseFields, requestedResource, m_current_config);
        } else {
            if (requestFields.count("Authorization") == 0 || requestFields.at("Authorization") != " Basic " + util::base64::convert_string(currentModule.authentication.first + ":" + currentModule.authentication.second) + "\r\n") {                
                responseFields["WWW-Authenticate"] = "Basic realm=\"" + search->second.name + "\"";
                result.second = 401;
            } else {    
                result = search->second.callback(requestFields, responseFields, requestedResource, m_current_config);                            
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



