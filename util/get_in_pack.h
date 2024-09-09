#pragma once

namespace util {
    template<int n, typename T, typename... Ts>
    const auto& get_in_pack(const T& arg, const Ts&... args) {
        if constexpr (n == 0) {
            return arg;
        } else {
            return get_in_pack<n - 1>(args...);
        }
    }
}
