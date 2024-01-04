#pragma once

namespace static_math {
namespace internal {
    template<typename T, T a, T b>
    concept is_smaller_or_equal = a <= b;
    
    template<typename T, T a, T b>
    concept is_smaller = a < b;
    
    template<typename T, T a, T b>
    concept is_equal = a == b;

    template<typename T, T b, T n, T v, T exp>
        requires is_smaller_or_equal<T, n, v>
    constexpr T log_check() {
        return exp;
    }

    template<typename T, T b, T n, T v, T exp>
        requires (!is_smaller_or_equal<T, n, v>)
    constexpr T log_check() {
        return log_check<T, b, n, b * v, exp + 1>();
    }
    
    template<typename T, T b, T n>
        requires is_equal<T, n, 0>
    constexpr T exp_check() {
        return 1;
    }

    template<typename T, T b, T n> 
        requires (!is_equal<T, n, 0>)
    constexpr T exp_check() {
        return b * exp_check<T, b, n - 1>();
    }       
    
    template<typename T, int n, int s>
        requires is_smaller_or_equal<T, sizeof(T) * 8, n * s>
    T bitbloat_check(const T v) {
        return 0;
    }

    template<typename T, int n, int s>
        requires (!is_smaller_or_equal<T, sizeof(T) * 8, n * s>)
    T bitbloat_check(const T v) {
        return warp_bit<T, n, n * s - n>(v) | bitbloat_check<T, n + 1, s>(v);
    }
}

template<typename T, T b, T n>
constexpr T log() {
    return internal::log_check<T, b, n, 1, 0>();
}

template<typename T, T b, T n> 
constexpr T exp() {
    return internal::exp_check<T, b, n>();
}

template<typename T, T a, T b> 
constexpr T round_up_division() {
    return a / b + ((a % b) > 0);
}    

template<typename T, int pos, int offset>
T warp_bit(const T v) {
    return (v & exp<T, 2, pos>()) << offset;
}

template<typename T, int s>
T bitbloat(const T v) {
    return internal::bitbloat_check<T, 0, s>(v);
}

template<typename T, T b, T x>
concept is_power_of = exp<T, b, log<T, b, x>()>() == x;

template<typename T, T f, T n>
concept is_factor_of = (n / f * f) == n;

template<unsigned max, unsigned min, typename T>
T bit_splice(const T v) {
    constexpr auto mask = (T(1) << (max - min + 1)) - 1;

    if constexpr (min >= sizeof(T) * 8) {
        return 0;
    } else if (max >= sizeof(T) * 8) {
        if constexpr (min == 0) {
            return v;
        } else {
            return (v >> min) & mask;
        }
    } else {
        return ((v << (sizeof(T) * 8 - max - 1)) >> (sizeof(T) * 8 - max - 1 + min)) & mask;
    }    
}

template<int size, int n = 0, typename T, typename Tcall>
bool call_on_subword(const T v, const Tcall& call) {
    if constexpr (n * size < sizeof(T) * 8) {
        if (call_on_subword<size, n + 1, T, Tcall>(v, call)) {
            return call(bit_splice<n * size + size - 1, n * size, T>(v), n * size);
        }
        return false;
    } else {
        return true;
    }
}
}

