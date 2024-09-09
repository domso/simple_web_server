#pragma once

#include <string_view>

#include "attribute.h"
#include "object.h"
#include "array.h"

#include "parser.h"

namespace json {
    template<typename T>
    class importer {
    public:
        importer() {
            m_grammar_parser.add_rule("ws", " *");
            m_grammar_parser.add_rule("number", "(((1-9)(0-9)*)|0)((.(0-9)(0-9)*(((e|E)(+|\\-|)(0-9)(0-9)*)|))|)");
            m_grammar_parser.add_rule("text", "\"((\x1-\x21)|(\x23-\\\x5B)|(\\\x5D-\xFF)|(\x5C\x5C(\x1-\xFF)))*\"");

            auto rules = T::rule_export(".top_obj", "");
            for (auto it = rules.rbegin(); it != rules.rend(); ++it) {
                const auto& [n, r] = *it;
                m_grammar_parser.add_rule(n, r);
            }
            m_grammar_parser.add_top_rule("top", "[.top_obj]");
        }
        
        void import(T& dest, const std::string_view& text) {
            dest.reset();
            m_name_stack.clear();
            m_node_stack.clear();

            m_name_stack.push_back("");
            m_node_stack.push_back(&dest);

            m_ignore = false;
            m_parse_number = false;
            m_parse_text = false;

            m_grammar_parser.parse(text, [&](const auto& scope) {
                open_scope(scope);
            }, [&](const auto& scope) {
                close_scope(scope);
            }, [&](const auto& scope) {
                read_scope(scope);
            });
        }
    private:
        void open_scope(const auto& scope) {
            if (scope == "ws") {
                m_ignore = true;
                return;
            }
            if (scope == "number") {
                m_parse_number = true;
                return;
            }
            if (scope == "text") {
                m_parse_text = true;
                return;
            }
            assert(m_name_stack.begin()->length() < scope.length());
            auto open_name = scope.substr(m_name_stack.rbegin()->length());

            if (open_name != ".top_obj") {
                assert(open_name.length() > 1);
                auto next = (*m_node_stack.rbegin())->get_child_by_name(open_name.substr(1));
                assert(next != nullptr);
                m_node_stack.push_back(next);
            }

            m_name_stack.push_back(scope);
        }
        void close_scope(const auto& scope) {
            if (scope == "ws") {
                m_ignore = false;
                return;
            }
            if (scope == "number") {
                m_parse_number = false;
                return;
            }
            if (scope == "text") {
                m_parse_text = false;
                return;
            }
            m_name_stack.pop_back();
            m_node_stack.pop_back();
        }
        void read_scope(const auto& scope) {
            if (!m_ignore) {
                if (m_parse_number) {
                    (*m_node_stack.rbegin())->set_number(scope);
                    return;
                }
                if (m_parse_text) {
                    (*m_node_stack.rbegin())->set_text(scope);
                    return;
                }
            }
        }

        parsery::parser m_grammar_parser;
        std::vector<std::string> m_name_stack;
        std::vector<json::parse_tree_node*> m_node_stack;
        bool m_ignore = false;
        bool m_parse_number = false;
        bool m_parse_text = false;
    };
}
