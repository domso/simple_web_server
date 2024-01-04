#pragma once

#include <type_traits>

namespace util {
    template<typename... T>
    class pack_instances {
    public:
        template<typename Treq>
        auto& get() {
        }

        template<typename Treq>
        bool has() {
            return false;
        }

        template<typename Tcall>
        void for_each(const Tcall& call) {
        }

        template<typename Tcall>
        void for_id(const size_t id, const Tcall& call) {
        }

        template<typename Treq>
        size_t id() {
            return 0;
        }
    private:
    };

    template<typename T, typename... Ts>
    class pack_instances<T, Ts...> {
    public:
        template<typename Treq>
        auto& get() {
            if constexpr (std::is_same<T, Treq>::value) {
                return local;
            } else {
                return others.template get<Treq>();
            }
        }

        template<typename Treq>
        bool has() {
            if constexpr (std::is_same<T, Treq>::value) {
                return true;
            } else {
                return others.template has<Treq>();
            }
        }

        template<typename Tcall>
        void for_each(const Tcall& call) {
            call.template operator()<T>(local);
            others.template for_each(call);
        }

        template<typename Tcall>
        void for_id(const size_t id, const Tcall& call) {
            if (id == 0) {
                call.template operator()<T>(local);
            } else {
                others.template for_id(id - 1, call);
            }
        }

        template<typename Treq>
        size_t id() {
            if constexpr (std::is_same<T, Treq>::value) {
                return 0;
            } else {
                return 1 + others.template id<Treq>();
            }
        }
    private:
        T local;
        pack_instances<Ts...> others;
    };
}
