#pragma once

#include <string>
#include <string_view>

namespace json {
    struct parse_tree_node {
        virtual parse_tree_node* get_child_by_name(const std::string& name) = 0;
        virtual void set_number(const std::string_view& number) = 0;
        virtual void set_text(const std::string_view& number) = 0;
    };
}
