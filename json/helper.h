#pragma once

#include <sstream>

namespace json {
    template<typename T>
    concept is_iterateable = requires(T a) {
        a.begin();
        a.end();
        a.size();
    };

    template<typename T>
    concept has_to_string = requires(T a, std::string t) {
        t = a.to_string();
    };

    template<typename T>
    concept is_convertable_to_string = requires(T a) {
        std::to_string(a);
    };

    template<typename T>
    concept has_rule_export = requires() {
        T::rule_export("", "");
    };

    template<typename T>
    concept has_clear = requires(T a) {
        a.clear();
    };

    template<typename T>
    concept is_settable_to_zero = requires(T a) {
        a = static_cast<T>(0);
    };

    template<typename T>
    T convert(const std::string str) {
        std::istringstream iss(str);
        T value;
        iss >> value;
        return value;
    }
}
