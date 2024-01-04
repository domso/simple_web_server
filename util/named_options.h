#pragma once

#include "util/named_type.h"
#include "util/pack_iteratable.h"

namespace util {
    template<typename T, typename... Ts>
    struct named_options {
        template<util::string_literal key, typename Tcall>
        static void get(const Tcall& call) {
            if constexpr (sizeof...(Ts) == 0) {
               if constexpr (T::name == key) {
                   call.template operator()<T::value>();
                   return;
               }
            } else {
               if constexpr (T::name == key) {
                   call.template operator()<T::value>();
                   return;
               } else {
                   return named_options<Ts...>::template get<key>(call);
               }
            }
        }
        template<util::string_literal key>
        static auto get_value() {
            if constexpr (sizeof...(Ts) == 0) {
               if constexpr (T::name == key) {
                   return T::value;
               }
            } else {
               if constexpr (T::name == key) {
                   return T::value;
               } else {
                   return named_options<Ts...>::template get_value<key>();
               }
            }
        }
    };
}
