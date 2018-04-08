#include "http_request.h"
#include "file_loader.h"

std::pair<std::unordered_map<std::string, std::string>, std::vector<char>> 
http_request::handle_request(const std::unordered_map<std::string, std::string>& requestFields, shared_context& context) {
    std::unordered_map<std::string, std::string> responseFields;
    std::vector<char> output = file_loader::load(filter_filename(requestFields.at("GET")));
    
    responseFields["Server"] = "MyServer";
    responseFields["Content-Length"] = std::to_string(output.size());
    responseFields["Connection"] = "keep-alive";
    
    if (output.empty()) {
        set_status(responseFields, context, 404);
    } else {
        set_status(responseFields, context, 200);
    }
    
    return std::make_pair(responseFields, output);
}

void http_request::set_status(std::unordered_map<std::string, std::string>& responseFields, shared_context& context, const int status) {
    responseFields["STATUS"] = std::to_string(status) + " " + context.statusCodes.get(status);
}

std::string http_request::filter_filename(const std::string& filename) {
    std::string result = filename;
        
    if (result.find("..") != std::string::npos) {
        result = "";
    }
    
    if (result == "/") {
        result = "/index.html";
    }
    
    return result;
}
