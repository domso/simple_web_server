#pragma once

#include <type_traits>
#include <vector>

#include "helper.h"

namespace json {
    template<typename T>
    using array=std::vector<T>;
}
