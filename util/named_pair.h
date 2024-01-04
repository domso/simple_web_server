#pragma once

#include "string_literal.h"

namespace util {
    template<util::string_literal name_, auto T_>
    struct named_pair {
        constexpr static const auto name = name_;
        constexpr static const auto value = T_;
    };
}

