#include "http_status_code.h"

http_status_code::http_status_code() {
    m_codes.insert_or_assign(100, "Continue");
    m_codes.insert_or_assign(101, "Switching Protocols");
    m_codes.insert_or_assign(102, "Processing");

    m_codes.insert_or_assign(200, "OK");
    m_codes.insert_or_assign(201, "Created");
    m_codes.insert_or_assign(202, "Accepted");
    m_codes.insert_or_assign(203, "Non-Authoritative Information");
    m_codes.insert_or_assign(204, "No Content");
    m_codes.insert_or_assign(205, "Reset Content");
    m_codes.insert_or_assign(206, "Partial Content");
    m_codes.insert_or_assign(207, "Multi-Status");
    m_codes.insert_or_assign(208, "Already Reported");
    m_codes.insert_or_assign(226, "IM Used");

    m_codes.insert_or_assign(300, "Multiple Choices");
    m_codes.insert_or_assign(301, "Moved Permanently");
    m_codes.insert_or_assign(302, "Found (Moved Temporarily)");
    m_codes.insert_or_assign(303, "See Other");
    m_codes.insert_or_assign(304, "Not Modified");
    m_codes.insert_or_assign(305, "Use Proxy");
    m_codes.insert_or_assign(307, "Temporary Redirect");
    m_codes.insert_or_assign(308, "Permanent Redirect");

    m_codes.insert_or_assign(400, "Bad Request");
    m_codes.insert_or_assign(401, "Unauthorized");
    m_codes.insert_or_assign(402, "Payment Required");
    m_codes.insert_or_assign(403, "Forbidden");
    m_codes.insert_or_assign(404, "Not Found");
    m_codes.insert_or_assign(405, "Method Not Allowed");
    m_codes.insert_or_assign(406, "Not Acceptable");
    m_codes.insert_or_assign(407, "Proxy Authentication Required");
    m_codes.insert_or_assign(408, "Request Timeout");
    m_codes.insert_or_assign(409, "Conflict");
    m_codes.insert_or_assign(410, "Gone");
    m_codes.insert_or_assign(411, "Length Required");
    m_codes.insert_or_assign(412, "Precondition Failed");
    m_codes.insert_or_assign(413, "Request Entity Too Large");
    m_codes.insert_or_assign(414, "URI Too Long");
    m_codes.insert_or_assign(415, "Unsupported Media Type");
    m_codes.insert_or_assign(416, "Requested range not satisfiable");
    m_codes.insert_or_assign(417, "Expectation Failed");
    m_codes.insert_or_assign(420, "Policy Not Fulfilled");
    m_codes.insert_or_assign(421, "Misdirected Request");
    m_codes.insert_or_assign(422, "Unprocessable Entity");
    m_codes.insert_or_assign(423, "Locked");
    m_codes.insert_or_assign(424, "Failed Dependency");
    m_codes.insert_or_assign(426, "Upgrade Required");
    m_codes.insert_or_assign(428, "Precondition Required");
    m_codes.insert_or_assign(429, "Too Many Requests");
    m_codes.insert_or_assign(431, "Request Header Fields Too Large");
    m_codes.insert_or_assign(451, "Unavailable For Legal Reasons");

    m_codes.insert_or_assign(500, "Internal Server Error");
    m_codes.insert_or_assign(501, "Not Implemented");
    m_codes.insert_or_assign(502, "Bad Gateway");
    m_codes.insert_or_assign(503, "Service Unavailable");
    m_codes.insert_or_assign(504, "Gateway Timeout");
    m_codes.insert_or_assign(505, "HTTP Version not supported");
    m_codes.insert_or_assign(506, "Variant Also Negotiates");
    m_codes.insert_or_assign(507, "Insufficient Storage");
    m_codes.insert_or_assign(508, "Loop Detected");
    m_codes.insert_or_assign(509, "Bandwidth Limit Exceeded");
    m_codes.insert_or_assign(510, "Not Extended");
    m_codes.insert_or_assign(511, "Network Authentication Required");
}

const std::string& http_status_code::get(const int status) {
    return m_codes[status];
}
