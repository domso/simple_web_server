#pragma once

#include "pack_instances.h"

namespace util {
    template<typename... Ts>
    struct pack_iteratable {
        template<typename Tcall>
        static void for_each(const Tcall& call) {
        }

        template<typename Tcall>
        static auto filter(const Tcall& call) {
            return pack_iteratable();
        }

        template<typename Tcall>
        static auto map(const Tcall& call) {
            return pack_iteratable();
        }

        typedef pack_instances<Ts...> instance;

        template<typename... Tothers>
        auto operator+(const pack_iteratable<Tothers...>& other) {
            return other;
        }

        template<int n>
        struct get {
            typedef std::false_type type;
        };
    };

    template<typename T, typename... Ts>
    struct pack_iteratable<T, Ts...> {
        template<typename Tcall>
        static void for_each(const Tcall& call) {
            call.template operator()<T>();
            pack_iteratable<Ts...>::template for_each(call);
        }

        template<typename Tcall>
        static constexpr auto filter(Tcall call) {
            if constexpr (call.template operator()<T>()) {
                auto current = pack_iteratable<T>();
                auto next = pack_iteratable<Ts...>::template filter(call);

                return current + next;
            } else {
                return pack_iteratable<Ts...>::template filter(call);
            }
        }

        template<typename Tcall>
        static constexpr auto map(Tcall call) {
            auto current = call.template operator()<T>();
            auto next = pack_iteratable<Ts...>::template map(call);

            return current + next;
        }

        template<typename... Tothers>
        auto operator+(const pack_iteratable<Tothers...>& other) {
            return pack_iteratable<T, Ts..., Tothers...>();
        }

        template<int n>
        struct get {
            typedef typename pack_iteratable<Ts...>::get<n - 1>::type type;
        };
        template<int n>
            requires (n == 0)
        struct get<n> {
            typedef T type;
        };

        typedef pack_instances<T, Ts...> instance;
    };
}
