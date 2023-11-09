#include "requester.h"

#include "util/logger.h"
#include "util/base64.h"

#include "web_server/http/parser.h"

web_server::http::requester::requester(config& current_config) : m_current_config(current_config) {}

std::pair<std::unordered_map<std::string, std::string>, std::vector<char>> 
web_server::http::requester::handle_request(const std::string& header, const std::shared_ptr<unique_context>& context) const {
    std::unordered_map<std::string, std::string> response_fields;
    
    response_fields["Server"] = m_current_config.name;
    response_fields["Connection"] = "keep-alive";
    
    if (auto request_fields = parser::parse_request(header)) {
        auto [output_content, output_code] = execute_callback(*request_fields, response_fields, context);

        response_fields["Content-Length"] = std::to_string(output_content.size());
        response_fields["STATUS"] = std::to_string(output_code) + " " + m_status_codes.get(output_code);
            
        return {response_fields, output_content};
    }

    response_fields["STATUS"] = std::to_string(400) + " " + m_status_codes.get(401);

    return {response_fields, {}};
}

std::pair<std::vector<char>, int> web_server::http::requester::execute_callback(const std::unordered_map<std::string, std::string>& request_fields, std::unordered_map<std::string, std::string>& response_fields, const std::shared_ptr<unique_context>& context) const {
    auto requested_resource = request_fields.at("#REQ").substr(0, request_fields.at("#REQ").find_first_of("?#"));
    auto requested_module = get_module(requested_resource);    
    auto requested_method = request_fields.at("#METHOD");
    auto updated_fields = insert_url_params(request_fields, request_fields.at("#REQ"));
    std::pair<std::vector<char>, int> result;
    
    result.second = 404;

    util::logger::log_debug("Requested ressource " + requested_resource);
    util::logger::log_debug("Requested module " + requested_module);

    auto search = m_module_map.find(requested_module);    
    if (search == m_module_map.end()) {
        search = m_module_map.find("/");          
    }
        
    if (search != m_module_map.end()) {
        const internal_module& current_module = search->second;
            
        if (current_module.authentication.first == "") {
            result = current_module.call_by_method(requested_method, updated_fields, response_fields, requested_resource, m_current_config);
        } else {
            if (request_fields.count("Authorization") == 0 || request_fields.at("Authorization") != " Basic " + util::base64::to_base64(current_module.authentication.first + ":" + current_module.authentication.second) + "\r\n") {                
                response_fields["WWW-Authenticate"] = "Basic realm=\"" + current_module.name + "\"";
                result.second = 401;
            } else {    
                result = current_module.call_by_method(requested_method, updated_fields, response_fields, requested_resource, m_current_config);                            
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

std::unordered_map<std::string, std::string> web_server::http::requester::insert_url_params(const std::unordered_map<std::string, std::string>& request_fields, const std::string& url) const {
    auto result = request_fields;
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
                        result[url.substr(matches[0].first, matches[0].second)] = url.substr(matches[1].first, matches[1].second);
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
            result[url.substr(matches[0].first, matches[0].second)] = url.substr(matches[1].first, matches[1].second);
        }
    }

    return result;
}
std::pair<std::vector<char>, int> web_server::http::requester::internal_module::call_by_method(
    const std::string& method,
    const std::unordered_map<std::string, std::string>& request_fields, 
    std::unordered_map<std::string, std::string>& response_fields, 
    const std::string& res, 
    const config& current_config
) const {
    if (method == "GET" && get_callback) {
        return get_callback(request_fields, response_fields, res, current_config);
    } else if (method == "PUT" && put_callback) {
        return put_callback(request_fields, response_fields, res, current_config);
    } else if (method == "POST" && post_callback) {
        return post_callback(request_fields, response_fields, res, current_config);
    } else if (method == "DELETE" && delete_callback) {
        return delete_callback(request_fields, response_fields, res, current_config);
    } else if (method == "PATCH" && patch_callback) {
        return patch_callback(request_fields, response_fields, res, current_config);
    } else if (method == "OPTIONS" && options_callback) {
        return options_callback(request_fields, response_fields, res, current_config);
    } else if (method == "TRACE" && trace_callback) {
        return trace_callback(request_fields, response_fields, res, current_config);
    } else if (method == "CONNECT" && connect_callback) {
        return connect_callback(request_fields, response_fields, res, current_config);
    }

    return {{}, 404};
}
