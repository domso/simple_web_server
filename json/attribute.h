#pragma once

#include <type_traits>
#include <string>
#include <string_view>
#include <vector>
#include <assert.h>

#include "util/string_literal.h"
#include "helper.h"
#include "parse_tree_node.h"

namespace json {
    template<util::string_literal name_, typename T>
    struct attribute : public parse_tree_node {
        constexpr static const auto name = name_;
        T data;
        typedef T type;
        bool valid = false;

        void reset() {
            valid = false;

            if constexpr (has_clear<T>) {
                data.clear();
            } else if constexpr (is_settable_to_zero<T>) {
                data = static_cast<T>(0);
            } else {
                data = T();
            }
        }

        parse_tree_node* get_child_by_name(const std::string& name) {
            if constexpr (std::is_base_of_v<parse_tree_node, T>) {
                return data.get_child_by_name(name);
            }
            if (name == "entry") {
                if constexpr (is_iterateable<T>) {
                    if constexpr (std::is_base_of_v<parse_tree_node, std::remove_cvref_t<decltype(*data.rbegin())>>) {
                        data.push_back({});
                        return &(*data.rbegin());
                    }
                }
            }
            if constexpr (is_iterateable<T>) {
                return this;
            }

            return nullptr;
        }
        void set_number(const std::string_view& number) {
            valid = true;
            if constexpr (std::is_arithmetic_v<T>) {
                data = convert<T>(std::string(number));
            }
            if constexpr (is_iterateable<T>) {
                typedef std::remove_cvref_t<decltype(*data.rbegin())> eT;

                if constexpr (std::is_arithmetic_v<eT>) {
                    data.push_back(convert<std::remove_cvref_t<decltype(*data.rbegin())>>(std::string(number)));
                }
            }
        }
        void set_text(const std::string_view& number) {
            valid = true;
            if constexpr (std::same_as<std::remove_cvref_t<T>, std::string>) {
                assert(number.length() >= 2);
                data.reserve(number.length() - 2);
                int n = 0;
                bool escape = false;
                for (auto c : number) {
                    if (0 < n && n < number.length() - 1) {
                        if (!escape) {
                            if (c == '\\') {
                                escape = true;
                            } else {
                                data += c;
                            }
                        } else {
                            data += c;
                            escape = false;
                        }
                    }
                    n++;
                }
            }
        }

        std::string to_string() const {
            std::string result = "\"";
            result += name.data();
            result += "\":";

            if constexpr (has_to_string<T>) {
                result += data.to_string();
            } else if constexpr (std::same_as<std::remove_cvref_t<T>, std::string>) {
                result.reserve(result.length() + 2 + data.length());
                result += "\"";
                for (auto c : data) {
                    if (c == '\\' || c == '"') {
                        result += "\\";
                    }
                    result += c;
                }
                result += "\"";
            } else if constexpr (is_iterateable<T>) {
                int n = 0;
                result += "[";
                for (const auto& v : data) {
                    if (n > 0) {
                        result += ",";
                    }
                    n++;

                    if constexpr (has_to_string<decltype(v)>) {
                        result += v.to_string();
                    } else if constexpr (std::same_as<std::remove_cvref_t<decltype(v)>, std::string>) {
                        result += "\"";
                        result += v;
                        result += "\"";
                    } else if constexpr (is_convertable_to_string<decltype(v)>) {
                        result += std::to_string(v);
                    }
                }
                result += "]";
            } else if constexpr (is_convertable_to_string<T>) {
                result += std::to_string(data);
            }

            return result;
        }
        static std::vector<std::pair<std::string, std::string>> rule_export(const std::string& prefix) {
            std::string rule_name = prefix + "." + name.data();
            std::string attr_prefix = std::string("\"") + name.data() + "\":[ws]";

            std::vector<std::pair<std::string, std::string>> rules;
            if constexpr (std::is_arithmetic_v<T>) {
                rules.push_back({
                    rule_name,
                    attr_prefix + "[number]"
                });
            } else if constexpr (std::same_as<std::remove_cvref_t<T>, std::string>) {
                rules.push_back({
                    rule_name,
                    attr_prefix + "[text]"
                });
            } else if constexpr (has_rule_export<T>) {
                for (const auto& r : T::rule_export(rule_name, attr_prefix)) {
                    rules.push_back(r);
                }
            } else if constexpr (is_iterateable<T>) {
                std::string entry = rule_name + ".entry";
                rules.push_back({
                    rule_name,
                    attr_prefix + "\\[[ws](([" + entry + "]([ws],[ws][" + entry +"])*)|)[ws]\\]"
                });
                typedef std::remove_cvref_t<decltype(*data.data())> element_type;

                if constexpr (std::is_arithmetic_v<element_type>) {
                    rules.push_back({
                        entry,
                        "[number]"
                    });
                } else if constexpr (std::same_as<std::remove_cvref_t<element_type>, std::string>) {
                    rules.push_back({
                        entry,
                        "[text]"
                    });
                } else if constexpr (has_rule_export<element_type>) {
                    element_type tmp;
                    for (const auto& r : tmp.rule_export(entry, "")) {
                        rules.push_back(r);
                    }
                }
            }

            return rules;
        }
    };
}
