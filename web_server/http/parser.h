#pragma once

#include <unordered_map>

#include "network/pkt_buffer.h"
#include "network/tcp_connection.h"

namespace web_server::http {
    class parser {
    public:
        static std::unordered_map<std::string, std::string> parse_request(const std::string& header);
    private:
        static void extract_header_option(std::unordered_map<std::string, std::string>& request_fields, const std::string& header, const int start, const int length);
        static std::string extract_requested_file(const std::string& header_field);
    };
}