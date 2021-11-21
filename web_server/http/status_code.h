#pragma once

#include <string>
#include <unordered_map>

namespace web_server::http {
    class status_code {
    public:
        status_code();
        const std::string& get(const int status) const;
    private:
        std::unordered_map<int, std::string> m_codes;
        std::string m_invalid_code = "Invalid status code";
    };
}
