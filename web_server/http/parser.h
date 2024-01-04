#pragma once

#include <unordered_map>
#include <optional>

#include "network/pkt_buffer.h"
#include "network/tcp_connection.h"

#include "request.h"
#include "response.h"

namespace web_server::http {
    class parser {
    public:
        static std::optional<http::request> parse_request(const std::string& header);
    private:
        static bool extract_method_option(http::request& request, const std::string& method);
        static void extract_header_option(http::request& request, const std::string& header, const int start, const int length);
        static std::string extract_requested_file(const std::string& header_field);

        static std::string get_module_by_name(const std::string& ressource);
        static void insert_url_params(http::request& request, const std::string& url);
    };
}
