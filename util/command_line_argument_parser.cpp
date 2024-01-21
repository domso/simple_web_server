#include "command_line_argument_parser.h"

#include <iostream>

#include <type_traits>                                                                                                                                                 
                                                                                                                                                                       
namespace web_server {
    command_line_argument_parser::command_line_argument_parser(const int argc, char** argv) : m_arguments(convert_arguments(argc, argv)) {}

    void command_line_argument_parser::add_help_text(const std::string& text) {
        m_help_text = text;
    }

    bool command_line_argument_parser::parse_arguments() {
        for (const auto& arg : m_arguments) {
            if (arg == "--help") {
                print_help();
                return false;
            }
        }

        m_current_argument = m_arguments.begin();

        while (m_current_argument != m_arguments.end()) {
            auto find = m_option_ids.find(*m_current_argument);
            if (find != m_option_ids.end()) {
                const auto& opt = m_options[find->second];
                m_current_argument++;

                if (!opt.call()) {
                    print_error("Invalid use of " + find->first);
                    return false;
                }
            } else {
                auto [prefix, postfix] = split(*m_current_argument);
                prefix += "=";
                find = m_option_ids.find(prefix);
                if (find != m_option_ids.end()) {
                    if (postfix != "") {
                        *m_current_argument=postfix;
                        m_current_argument = m_arguments.insert(m_current_argument, prefix);
                    } else {
                        *m_current_argument=prefix;
                    }
                } else {
                    print_error("Unknown option '" + *m_current_argument + "'");
                    return false;
                }
            }
        }

        for (const auto& opt : m_options) {
            if (opt.is_required) {
                print_error("Option " + opt.first_option + " is required");
                return false;
            }
        }

        return true;
    }

    std::pair<std::string, std::string> command_line_argument_parser::split(const std::string& full) const {
        auto find = full.find_first_of("=");
        if (find != std::string::npos) {
            return {full.substr(0, find), full.substr(find + 1)};
        } else {
            return {full, ""};
        }
    }

    void command_line_argument_parser::print_error(const std::string& error) const {
        std::cout << m_executable << ": " << error << "!\nTry '" + m_executable + " --help' for more informations" << std::endl;
    }

    void command_line_argument_parser::print_help() const {
        std::cout << m_help_text << std::endl;
        {
            table_converter conv;

            conv.add_cell("Required");
            conv.add_cell("Alternatives");
            conv.add_cell("Arguments");
            conv.add_cell("Description");
            conv.close_row();
        
            for (const auto& opt : m_options) {
                if (opt.is_required) {
                    conv.add_cell(opt.first_option);
                    conv.add_cell(opt.alternatives);
                    conv.add_cell(opt.arguments);
                    conv.add_cell(opt.description);
                    conv.close_row();
                }
            }

            std::cout << conv.table() << std::endl;
        }
        {
            table_converter conv;

            conv.add_cell("Optional");
            conv.add_cell("Alternatives");
            conv.add_cell("Arguments");
            conv.add_cell("Description");
            conv.close_row();
        
            for (const auto& opt : m_options) {
                if (!opt.is_required) {
                    conv.add_cell(opt.first_option);
                    conv.add_cell(opt.alternatives);
                    conv.add_cell(opt.arguments);
                    conv.add_cell(opt.description);
                    conv.close_row();
                }
            }

            std::cout << conv.table() << std::endl;
        }
    }

    std::vector<std::string> command_line_argument_parser::convert_arguments(const int argc, char** argv) {
        std::vector<std::string> result;

        if (argc > 0) {
            m_executable = std::string(argv[0]);
        }

        for (int i = 1; i < argc; i++) {
            result.push_back(std::string(argv[i]));
        }

        return result;
    }
}
