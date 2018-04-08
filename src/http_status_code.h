#pragma once

#include <string>
#include <unordered_map>

class http_status_code {
public:
    http_status_code();
    const std::string& get(const int status);
private:
    std::unordered_map<int, std::string> m_codes;
};
