#include "parser.h"

std::optional<web_server::http::request> web_server::http::parser::parse_request(const std::string& header) {   
    http::request result;

    int current_position = 0;
    for (int i = 1; i < header.length(); i++) {
        if (header[i - 0] == '\n' &&
            header[i - 1] == '\r'
        ) {
            extract_header_option(result, header, current_position, i - current_position + 1);
            current_position = i + 1;
        }
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

bool web_server::http::parser::extract_method_option(http::request& request, const std::string& method) {
    if (auto it = request.fields.find(method) ; it != request.fields.end()) {
        auto file = extract_requested_file(request.fields[method]);
        request.resource = file.substr(0, file.find_first_of("?#"));
        request.module = get_module_by_name(request.resource);
        request.method = method;
        
        insert_url_params(request, file);

        return file != "";
    }
    
    return false;
}

void web_server::http::parser::extract_header_option(http::request& request, const std::string& header, const int start, const int length) {
    for (int i = 0; i < length; i++) {
        if (header[start + i] == ':' || header[start + i] == ' ') {
            request.fields[header.substr(start, i)] = header.substr(start + i + 1, length - i - 1);
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

std::string web_server::http::parser::get_module_by_name(const std::string& ressource) {
    auto pos = ressource.find_first_of("/", 1);
    
    if (pos == std::string::npos) {
        return "/";
    } else {
        return ressource.substr(0, pos);
    }    
}

void web_server::http::parser::insert_url_params(http::request& request, const std::string& url) {
    auto pos = url.find_first_of("?", 1);

    bool escaped = false;
    std::string pattern = "=;";
    std::vector<std::pair<size_t, size_t>> matches(2);
    size_t ptr = 0;

    if (pos != std::string::npos && pos + 1 != std::string::npos) {
        size_t current = pos + 1;

        matches[0] = {current, 0};
        matches[1] = {0, 0};
        for (const auto& c : url.substr(pos + 1)) {
            if (!escaped) {
                if (c == '\\') {
                    escaped = !escaped;
                }

                if (c == pattern[ptr]) {
                    ptr++;

                    if (ptr == pattern.length()) {
                        request.parameter[url.substr(matches[0].first, matches[0].second)] = url.substr(matches[1].first, matches[1].second);
                        matches[0] = {current + 1, 0};
                        matches[1] = {0, 0};
                        ptr = 0;
                    } else {
                        matches[ptr] = {current + 1, 0};
                    }
                } else {
                    matches[ptr].second++;    
                }
            }

            current++;
        }
        if (ptr == 1 && matches[0].first < url.length() && matches[1].first < url.length()) {
            request.parameter[url.substr(matches[0].first, matches[0].second)] = url.substr(matches[1].first, matches[1].second);
        }
    }
}
