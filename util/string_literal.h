#pragma once

#include <type_traits>
#include <array>
#include <algorithm>

#include "static_math.h"
#include "string.h"

namespace std {
    // why on eath is this function unclear??? like how could you possible define it different?????
    constexpr size_t abs(const size_t a) {
        return a;
    }
}

namespace util {

template<int N>
struct string_literal {
    constexpr string_literal(const char (&s)[N]) {
        std::copy_n(s, N, value.data());
    }
    constexpr string_literal(const char c) {
        for (int i = 0; i < N - 1; i++) {
            value[i] = c;
        }
        value[N - 1] = '\0';
    }

    constexpr int length() const {
        return N - 1;
    }

    constexpr char at(const int n) const {
        return value[n];
    }

    constexpr const char* data() const {
        return value.data();
    }

    constexpr string_literal() { }

    constexpr bool check(const char* s) const {     
        for (int i = 0; i < N; i++) {
            if (value[i] != s[i]) {
                return false;
            }
        }

        return true;
    }

    template<int M>
    constexpr bool operator==(const string_literal<M>& other) const {
        if constexpr (M == N) {
            return value == other.value;
        } else {
            return false;
        }
    }

    template<int M>
    constexpr string_literal<N + M - 1> operator+(const string_literal<M>& other) const {
        string_literal<N + M - 1> result;

        for (int i = 0; i < N - 1; i++) {
            result.value[i] = value[i];
        }

        for (int i = 0; i < M; i++) {
            result.value[N - 1 + i] = other.value[i];
        }

        return result;
    }

    std::array<char, N> value;
};

template<auto n, size_t precision = 3>
constexpr auto to_string_literal() {
    if constexpr (n >= 0) {
        if constexpr (std::is_integral<decltype(n)>::value) {
                if constexpr (n == 0) {
                    return string_literal("0");
                } else {
                    char d[2];
                    d[0] = 48 + n % 10;
                    d[1] = '\0';

                    if constexpr ((n / 10) > 0) {
                        return to_string_literal<n / 10, precision>() + string_literal(d);
                    } else {
                        return string_literal(d);
                    }
                }
        } else {
            // divide the number into <a>.<b><c>
            constexpr size_t a = static_cast<size_t>(n);
            constexpr decltype(n) bc = (n - a) * static_math::exp<decltype(n), 10, precision>();
            constexpr size_t b = static_cast<size_t>(bc);
            constexpr decltype(n) c = bc - b;

            if constexpr (c >= 0.5) {              
                if constexpr ((b + 1) >= static_math::exp<size_t, 10, precision>()) {
                    auto a_text = to_string_literal<a + 1, precision>();
                    auto b_text = to_string_literal<b + 1 - static_math::exp<size_t, 10, precision>(), precision>();
                    
                    return a_text + string_literal(".") + b_text;
                } else {
                    auto a_text = to_string_literal<a, precision>();
                    auto b_text = to_string_literal<b + 1, precision>();
                    
                    return a_text + string_literal(".") + b_text;
                }     
            } else {
                auto a_text = to_string_literal<a, precision>();
                auto b_text = to_string_literal<b, precision>();
                
                return a_text + string_literal(".") + b_text;
            } 
        }
    } else {
        return string_literal("-") + to_string_literal<std::abs(n), precision>();
    }
}


}
