#pragma once

#include <type_traits>
#include <string>
#include <string_view>
#include <vector>
#include <assert.h>

#include "helper.h"
#include "parse_tree_node.h"
#include "util/pack_instances.h"
#include "util/pack_iteratable.h"
#include "util/get_in_pack.h"

namespace json {
    template<typename... Ts>
    struct object : public parse_tree_node {
        util::pack_instances<Ts...> packs;

        object() {}
        object(const Ts::type&... args) {
            assign<0>(args...);
        }

        void reset() {
            packs.template for_each([&](auto& v) {
                v.reset();
            });
        }

        parse_tree_node* get_child_by_name(const std::string& name) {
            parse_tree_node* result = nullptr;
            packs.template for_each([&](auto& v) {
                if (v.name.check(name.c_str())) {
                    result = &v;
                }
            });
            return result;
        }
        void set_number(const std::string_view& number) {}
        void set_text(const std::string_view& number) {}

        template<util::string_literal name, util::string_literal... names>
        auto& child() {
            typedef typename decltype(util::pack_iteratable<Ts...>::filter([&]<typename T>() constexpr {
                return T::name == name;
            }))::get<0>::type type_of_name;

            if constexpr (sizeof...(names) == 0) {
                return packs.template get<type_of_name>();
            } else {
                return packs.template get<type_of_name>().data.template child<names...>();
            }
        }
        template<util::string_literal name, util::string_literal... names>
        const auto& child() const {
            typedef typename decltype(util::pack_iteratable<Ts...>::filter([&]<typename T>() constexpr {
                return T::name == name;
            }))::get<0>::type type_of_name;

            if constexpr (sizeof...(names) == 0) {
                return packs.template get<type_of_name>();
            } else {
                return packs.template get<type_of_name>().data.template child<names...>();
            }
        }
        template<util::string_literal name, util::string_literal... names>
        auto& at() {
            return child<name, names...>().data;
        }
        template<util::string_literal name, util::string_literal... names>
        const auto& at() const {
            return child<name, names...>().data;
        }
        template<util::string_literal name, util::string_literal... names>
        bool has() {
            return child<name, names...>().valid;
        }

        template<int n>
        void assign(const Ts::type&... args) {
            if constexpr (n < sizeof...(args)) {
                packs.template get<typename util::pack_iteratable<Ts...>::get<n>::type>().data = util::get_in_pack<n>(args...);
                packs.template get<typename util::pack_iteratable<Ts...>::get<n>::type>().valid = true;
                assign<n + 1>(args...);
            }
        }

        std::string to_string() const {
            std::string result;

            result += "{";
            int n = 0;

            packs.template for_each([&](const auto& v) {
                if (n == 0) {
                    result += v.to_string();
                } else {
                    result += ", " + v.to_string();
                }
                n++;
            });
            
            result += "}";

            return result;
        }

        static std::vector<std::pair<std::string, std::string>> rule_export(const std::string& prefix, const std::string& label) {
            std::vector<std::pair<std::string, std::string>> rules;

            std::string rule = "(";

            int n = 0;
            util::pack_iteratable<Ts...>::for_each([&]<typename T>() {
                if (n == 0) {
                    rule += "[" + prefix + "." + T::name.data() + "]";
                } else {
                    rule += "|[" + prefix + "." + T::name.data() + "]";
                }
                n++;
            });
            rule += ")";

            rules.push_back({
                prefix,
                label + "[ws]{[ws](((" + rule + ")([ws],[ws](" + rule + "))*)|)[ws]}[ws]"
            });
            util::pack_iteratable<Ts...>::for_each([&]<typename T>() {
                for (const auto& r : T::rule_export(prefix)) {
                    rules.push_back(r);
                }
            });

            return rules;
        }
    };
}
