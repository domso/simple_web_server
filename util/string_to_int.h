#pragma once

#include <optional>
#include <string>

namespace web_server {
template<typename T>
    std::optional<T> string_to_int(const std::string& str) {
        try {
            size_t pos;
            T result;

            // Check if the string starts with "0x" indicating hexadecimal format
            if (str.compare(0, 2, "0x") == 0) {
                result = std::stoull(str.substr(2), &pos, 16); // Convert as hexadecimal
                pos += 2;
            } else {
                result = std::stoull(str, &pos); // Convert as decimal
            }

            // Make sure the entire string was consumed
            if (pos != str.length()) {
                return std::nullopt; // Conversion failed, return empty optional
            }

            return result; // Conversion succeeded, return the result
        } catch (const std::exception&) {
            return std::nullopt; // Conversion failed, return empty optional
        }
    }
}
