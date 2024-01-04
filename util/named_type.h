#pragma once

#include "string_literal.h"

namespace util {
    template<util::string_literal name_, typename T_>
    struct named_type {
        constexpr static const auto name = name_;
        typedef T_ type;
    };
}
