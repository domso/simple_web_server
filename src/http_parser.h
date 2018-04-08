#pragma once

#include <unordered_map>
#include "network/pkt_buffer.h"
#include "network/tcp_connection.h"
#include "shared_context.h"

class http_parser {
public:
    static std::unordered_map<std::string, std::string> parse_request(const std::string& header);
private:
    static void extract_header_option(std::unordered_map<std::string, std::string>& requestFields, const std::string& header, const int start, const int length);
    static std::string extract_requested_file(const std::string& headerField);
};
