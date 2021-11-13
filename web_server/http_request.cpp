#include "http_request.h"
#include <iostream>
#include "../util/base64.h"

web_server::http_request::http_request(web_server::shared_context& context) : m_context(context) {
}

std::pair<std::unordered_map<std::string, std::string>, std::vector<char>> 
web_server::http_request::handle_request(const std::unordered_map<std::string, std::string>& requestFields) {
    std::unordered_map<std::string, std::string> responseFields;
    
    responseFields["Server"] = m_context.currentConfig.name;
    responseFields["Connection"] = "keep-alive";
    
    std::pair<std::vector<char>, int> output = execute_callback(requestFields, responseFields);

    responseFields["Content-Length"] = std::to_string(output.first.size());
    responseFields["STATUS"] = std::to_string(output.second) + " " + m_context.statusCodes.get(output.second);
        
    return std::make_pair(responseFields, output.first);
}

std::pair<std::vector<char>, int> web_server::http_request::execute_callback(const std::unordered_map<std::string, std::string>& requestFields, std::unordered_map<std::string, std::string>& responseFields) {
    std::string requestedResource = requestFields.at("GET").substr(0, requestFields.at("GET").find_first_of("?#"));
    std::string requestedModule = get_module(requestedResource);    
    std::pair<std::vector<char>, int> result;
    
    result.second = 404;

    auto search = m_context.moduleMap.find(requestedModule);    
        
    if (search != m_context.moduleMap.end()) {
        const module_context& currentModule = search->second;
            
        if (currentModule.authentication.first == "") {
            result = search->second.callback(requestFields, responseFields, requestedResource, m_context.currentConfig);
        } else {
            if (requestFields.count("Authorization") == 0 || requestFields.at("Authorization") != " Basic " + util::base64::convert_string(currentModule.authentication.first + ":" + currentModule.authentication.second) + "\r\n") {                
                responseFields["WWW-Authenticate"] = "Basic realm=\"" + search->second.name + "\"";
                result.second = 401;
            } else {    
                result = search->second.callback(requestFields, responseFields, requestedResource, m_context.currentConfig);                            
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



