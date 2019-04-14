#include "http_status_code.h"

web_server::http_status_code::http_status_code() {
    m_codes.insert({100, "Continue"});
    m_codes.insert({101, "Switching Protocols"});
    m_codes.insert({102, "Processing"});

    m_codes.insert({200, "OK"});
    m_codes.insert({201, "Created"});
    m_codes.insert({202, "Accepted"});
    m_codes.insert({203, "Non-Authoritative Information"});
    m_codes.insert({204, "No Content"});
    m_codes.insert({205, "Reset Content"});
    m_codes.insert({206, "Partial Content"});
    m_codes.insert({207, "Multi-Status"});
    m_codes.insert({208, "Already Reported"});
    m_codes.insert({226, "IM Used"});

    m_codes.insert({300, "Multiple Choices"});
    m_codes.insert({301, "Moved Permanently"});
    m_codes.insert({302, "Found (Moved Temporarily)"});
    m_codes.insert({303, "See Other"});
    m_codes.insert({304, "Not Modified"});
    m_codes.insert({305, "Use Proxy"});
    m_codes.insert({307, "Temporary Redirect"});
    m_codes.insert({308, "Permanent Redirect"});

    m_codes.insert({400, "Bad Request"});
    m_codes.insert({401, "Unauthorized"});
    m_codes.insert({402, "Payment Required"});
    m_codes.insert({403, "Forbidden"});
    m_codes.insert({404, "Not Found"});
    m_codes.insert({405, "Method Not Allowed"});
    m_codes.insert({406, "Not Acceptable"});
    m_codes.insert({407, "Proxy Authentication Required"});
    m_codes.insert({408, "Request Timeout"});
    m_codes.insert({409, "Conflict"});
    m_codes.insert({410, "Gone"});
    m_codes.insert({411, "Length Required"});
    m_codes.insert({412, "Precondition Failed"});
    m_codes.insert({413, "Request Entity Too Large"});
    m_codes.insert({414, "URI Too Long"});
    m_codes.insert({415, "Unsupported Media Type"});
    m_codes.insert({416, "Requested range not satisfiable"});
    m_codes.insert({417, "Expectation Failed"});
    m_codes.insert({420, "Policy Not Fulfilled"});
    m_codes.insert({421, "Misdirected Request"});
    m_codes.insert({422, "Unprocessable Entity"});
    m_codes.insert({423, "Locked"});
    m_codes.insert({424, "Failed Dependency"});
    m_codes.insert({426, "Upgrade Required"});
    m_codes.insert({428, "Precondition Required"});
    m_codes.insert({429, "Too Many Requests"});
    m_codes.insert({431, "Request Header Fields Too Large"});
    m_codes.insert({451, "Unavailable For Legal Reasons"});

    m_codes.insert({500, "Internal Server Error"});
    m_codes.insert({501, "Not Implemented"});
    m_codes.insert({502, "Bad Gateway"});
    m_codes.insert({503, "Service Unavailable"});
    m_codes.insert({504, "Gateway Timeout"});
    m_codes.insert({505, "HTTP Version not supported"});
    m_codes.insert({506, "Variant Also Negotiates"});
    m_codes.insert({507, "Insufficient Storage"});
    m_codes.insert({508, "Loop Detected"});
    m_codes.insert({509, "Bandwidth Limit Exceeded"});
    m_codes.insert({510, "Not Extended"});
    m_codes.insert({511, "Network Authentication Required"});
}

const std::string& web_server::http_status_code::get(const int status) {
    return m_codes[status];
}
