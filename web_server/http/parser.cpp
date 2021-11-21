#include "parser.h"

std::unordered_map<std::string, std::string> web_server::http::parser::parse_request(const std::string& header) {   
    std::unordered_map<std::string, std::string> result;

    int current_position = 0;
    for (int i = 1; i < header.length(); i++) {
        if (header[i - 0] == '\n' &&
            header[i - 1] == '\r'
        ) {
            extract_header_option(result, header, current_position, i - current_position + 1);
            current_position = i + 1;
        }
    }

    result["GET"] = extract_requested_file(result["GET"]);
    
    return result;
}

void web_server::http::parser::extract_header_option(std::unordered_map<std::string, std::string>& request_fields, const std::string& header, const int start, const int length) {
    for (int i = 0; i < length; i++) {
        if (header[start + i] == ':' || header[start + i] == ' ') {
            request_fields[header.substr(start, i)] = header.substr(start + i + 1, length - i - 1);
        }
    }
}

std::string web_server::http::parser::extract_requested_file(const std::string& header_field) {
    int position = header_field.find(" ");
    std::string result;
    
    if (position != std::string::npos) {
        result = header_field.substr(0, position);
    }
    
    return result;
}


