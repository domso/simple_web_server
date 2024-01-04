#pragma once

namespace util {
    template<typename T, typename... Ts>
    struct tagged_type {
        T instance;
    };
}

