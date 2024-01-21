#pragma once

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

#include "table_converter.h"

namespace web_server {
    class command_line_argument_parser {
    public:
        command_line_argument_parser(const int argc, char** argv);

        template<typename T, typename... Ts, typename Tcall>
        void add_optional(const std::string& description, const Tcall call, const T first_option, const Ts... options) {
            auto opt = build_labels(description, false, number_of_expected_args<0, std::string>(call), first_option, options...);
            opt.call = [&, id = m_options.size(), local_call=call]() {
                m_options[id].is_required = false;
                std::vector<std::string>::iterator end = m_arguments.end();
                bool result = call_with_expected_args<0, std::string>(local_call, m_current_argument, end);
                return result;
            };

            opt.is_required = false;
            m_options.push_back(opt);

            for (const auto option : {first_option, options...}) {
                m_option_ids[option] = {
                    m_options.size() - 1,
                };
            }
        }

        template<typename... Ts, typename Tcall>
        void add_required(const std::string& description, const Tcall call, const Ts... options) {
            add_optional(description, call, options...);
            m_options.rbegin()->is_required = true;
        }

        void add_help_text(const std::string& text);

        bool parse_arguments();
    private:
        template<int n, typename Targ, typename T, typename... Ts>
        int number_of_expected_args(const T& call, Ts... args) {
            if constexpr (std::is_invocable<T, Ts...>()) {
                return 0;
            } else {
                Targ arg;
                return 1 + number_of_expected_args<n + 1, Targ, T, Targ, Ts...>(call, args..., arg);
            }
        }

        template<int n, typename Targ, typename Titerator, typename T, typename... Ts>
        bool call_with_expected_args(const T& call, Titerator& it, Titerator& end, Ts... args) {
            if constexpr (std::is_invocable<T, Ts...>()) {
                return call(args...);
            } else {
                if (it != end) {
                    Targ arg = *it;
                    it++;
                    return call_with_expected_args<n + 1, Targ, Titerator, T, Targ, Ts...>(call, it, end, args..., arg);
                } else {
                    return false;
                }
            }
        }

        struct loaded_option {
            std::string description;
            std::string first_option;
            std::string alternatives;
            std::string arguments;

            std::function<bool()> call;
            bool is_required;
        };

        template<typename T, typename... Ts>
        loaded_option build_labels(const std::string& description, const bool optional, const int number_of_args, const T& first_option, const Ts&... options) {
            loaded_option labels;
            labels.description = description;
            labels.first_option = first_option;

            if constexpr (sizeof...(options) > 0) {
                for (const auto option : {options...}) {
                    if (labels.alternatives != "") {
                        labels.alternatives += "\n";
                    }
                    labels.alternatives += option;
                }
            }

            for (int i = 0; i < number_of_args; i++) {
                if (labels.arguments != "") {
                    labels.arguments += " ";
                }
                char c = 97 + i;
                labels.arguments += "<";
                labels.arguments += c;
                labels.arguments += ">";
            }

            return labels;
        }

        std::pair<std::string, std::string> split(const std::string& full) const;
        void print_error(const std::string& error) const;
        void print_help() const;
        std::vector<std::string> convert_arguments(const int argc, char** argv);

        std::string m_executable;
        std::string m_help_text;
        std::vector<std::string> m_arguments;
        std::vector<std::string>::iterator m_current_argument;
        std::unordered_map<std::string, size_t> m_option_ids;

        std::vector<loaded_option> m_options;
    };
}
