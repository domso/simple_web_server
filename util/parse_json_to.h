#pragma once

#include <string>

namespace util {
    template<typename T>
    concept json_attribute_settable = requires(T a, const std::string_view& key, const std::string_view& value) {
        a.set_attribute(key, value);
    };

    template<typename T>
    T parse_json_to(const std::string_view& json) {
        T result;
        bool escaped = false;

        std::string sequence = "{\"\":\"\",}";
        constexpr auto escape_symbol = '\\';
        constexpr size_t optional_branch_start_index = 6;
        constexpr size_t optional_branch_target_index = 1;
        
        constexpr size_t key_start_index = 1;
        constexpr size_t key_end_index = 2;
        constexpr size_t value_start_index = 4;
        constexpr size_t value_end_index = 5;
        
        size_t current_symbol = 0;
        size_t key_start, key_end, value_start, value_end;

        for (size_t i = 0; i < json.length(); i++) {
            auto current = json.at(i);
            
            if (!escaped) {
                if (current == sequence[current_symbol]) {
                    auto old_symbol = current_symbol;
                    current_symbol = (current_symbol + 1) % sequence.length();
                    switch (old_symbol) {
                        case key_start_index: {
                            key_start = i;
                            break;
                        }
                        case key_end_index: {
                            key_end = i; 
                            break;
                        }
                        case value_start_index: {
                            value_start = i; 
                            break;
                        }
                        case value_end_index: {
                            value_end = i; 
                            result.set_attribute(json.substr(key_start + 1, key_end - key_start - 1), json.substr(value_start + 1, value_end - value_start - 1));
                            break;
                        }
                        case optional_branch_start_index: {
                            current_symbol = optional_branch_target_index; 
                            break;
                        }
                        default: {
                            break;
                        }
                    }

                } else if (current_symbol == optional_branch_start_index && current == sequence[current_symbol + 1]) {
                    current_symbol = (current_symbol + 2) % sequence.length();
                }

                escaped = (current == escape_symbol);
            } else {
                escaped = false;
            }
        }

        return result;
    }
}
