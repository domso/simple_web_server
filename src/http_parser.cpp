#include "http_parser.h"

std::unordered_map<std::string, std::string> http_parser::parse_request(const std::string& header) {   
    std::unordered_map<std::string, std::string> result;

    int currentPosition = 0;
    for (int i = 1; i < header.length(); i++) {
        if (header[i - 0] == '\n' &&
            header[i - 1] == '\r'
        ) {
            extract_header_option(result, header, currentPosition, i - currentPosition + 1);
            currentPosition = i + 1;
        }
    }

    result["GET"] = extract_requested_file(result["GET"]);
    
    return result;
}

void http_parser::extract_header_option(std::unordered_map<std::string, std::string>& requestFields, const std::string& header, const int start, const int length) {
    for (int i = 0; i < length; i++) {
        if (header[start + i] == ':' || header[start + i] == ' ') {
            requestFields[header.substr(start, i)] = header.substr(start + i + 1, length - i - 1);
        }
    }
}

std::string http_parser::extract_requested_file(const std::string& headerField) {
    int position = headerField.find(" ");
    std::string result;
    
    if (position != std::string::npos) {
        result = headerField.substr(0, position);
    }
    
    return result;
}


