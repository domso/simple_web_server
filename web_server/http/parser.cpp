#include "parser.h"

std::optional<std::unordered_map<std::string, std::string>> web_server::http::parser::parse_request(const std::string& header) {   
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

    if (result.contains("#REQ") || result.contains("#METHOD")) {
        return std::nullopt;
    }

    if (extract_method_option(result, "GET")) {
        return result;
    }
    if (extract_method_option(result, "PUT")) {
        return result;
    }
    if (extract_method_option(result, "POST")) {
        return result;
    }
    if (extract_method_option(result, "DELETE")) {
        return result;
    }
    if (extract_method_option(result, "CONNECT")) {
        return result;
    }
    if (extract_method_option(result, "OPTIONS")) {
        return result;
    }
    if (extract_method_option(result, "TRACE")) {
        return result;
    }
    if (extract_method_option(result, "PATCH")) {
        return result;
    }
    
    return std::nullopt;
}

bool web_server::http::parser::extract_method_option(std::unordered_map<std::string, std::string>& request_fields, const std::string& method) {
    if (auto it = request_fields.find(method) ; it != request_fields.end()) {
        auto file = extract_requested_file(request_fields[method]);
        request_fields["#REQ"] = file;
        request_fields["#METHOD"] = method;

        return file != "";
    }
    
    return false;
}

void web_server::http::parser::extract_header_option(std::unordered_map<std::string, std::string>& request_fields, const std::string& header, const int start, const int length) {
    for (int i = 0; i < length; i++) {
        if (header[start + i] == ':' || header[start + i] == ' ') {
            request_fields[header.substr(start, i)] = header.substr(start + i + 1, length - i - 1);
            return;
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


