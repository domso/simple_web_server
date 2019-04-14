#include "http_request.h"
#include "file_loader.h"

std::pair<std::unordered_map<std::string, std::string>, std::vector<char>> 
web_server::http_request::handle_request(const std::unordered_map<std::string, std::string>& requestFields, shared_context& context) {
    std::unordered_map<std::string, std::string> responseFields;
    
    std::pair<std::vector<char>, int> output = execute_callback(requestFields, context);
    
    responseFields["Server"] = context.currentConfig.name;
    responseFields["Content-Length"] = std::to_string(output.first.size());
    responseFields["Connection"] = "keep-alive";
    responseFields["STATUS"] = std::to_string(output.second) + " " + context.statusCodes.get(output.second);
    
    return std::make_pair(responseFields, output.first);
}

std::pair<std::vector<char>, int> web_server::http_request::execute_callback(const std::unordered_map<std::string, std::string>& requestFields, const shared_context& context) {
    std::string requestedResource = requestFields.at("GET").substr(0, requestFields.at("GET").find_first_of("?#"));
    std::pair<std::vector<char>, int> result;
    result.second = 404;
    
    auto search = context.callbackMap.find(requestedResource);    
    if (search != context.callbackMap.end()) {
        result = search->second(nullptr);
    } else {
        if (context.currentConfig.allow_file_access) {
            result.first = file_loader::load(filter_filename(requestFields.at("GET")), context.currentConfig);
            if (!result.first.empty()) {
                result.second = 200;
            }
        }
    }
    
    return result;
}

std::string web_server::http_request::filter_filename(const std::string& filename) {
    std::string result = filename;
        
    if (result.find("..") != std::string::npos) {
        result = "";
    }
    
    if (result == "/") {
        result = "/index.html";
    }
    
    return result;
}
