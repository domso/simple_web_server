#pragma once

#include <iostream>
#include <type_traits>
#include <chrono>
#include <mutex>

namespace util {
    template<typename T>
    concept is_printable = requires(T t) {
        std::cout << t;
    };
    template<typename T>
    concept is_elementwise_printable = requires(T t) {
        t.begin();
        t.end();
        t.begin()++;
        std::cout << *t.begin();
    };
    template<typename T>
    concept is_element_pairwise_printable = requires(T t) {
        t.begin();
        t.end();
        t.begin()++;
        std::cout << t.begin()->first;
        std::cout << t.begin()->second;
    };
        
    class logger {
    public:
        template<typename T>
        static void init() {
            m_enable_log_error = T::template get_value<"enable_log_error">();
            m_enable_log_warning = T::template get_value<"enable_log_warning">();
            m_enable_log_status = T::template get_value<"enable_log_status">();
            m_enable_log_debug = T::template get_value<"enable_log_debug">();
        }
        
        template<typename T>
        static void log_status(const T& msg) {
            if (m_enable_log_status) {
                print_as_string<T>(current_date() + " [STATUS] ", msg);
            }
        }

        template<typename T>
        static void log_warning(const T& msg) {
            if (m_enable_log_warning) {
                print_as_string<T>(current_date() + " \033[33m[WARNING]\033[0m ", msg);
            }
        }

        template<typename T>
        static void log_error(const T& msg) {
            if (m_enable_log_error) {
                print_as_string<T>(current_date() + " \033[31m[ERROR]\033[0m ", msg);
            }
        }

        template<typename T>
        static void log_debug(const T& msg) {
            if (m_enable_log_debug) {
                print_as_string<T>(current_date() + " \033[36m[DEBUG]\033[0m ", msg);
            }
        }
    private:                
        static bool m_enable_log_status;
        static bool m_enable_log_warning;
        static bool m_enable_log_error;
        static bool m_enable_log_debug;
        static std::mutex m_logger_mutex;
        
        static std::string current_date() {
            auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            auto result = std::string(std::ctime(&t));            
            return result.substr(0, result.length() - 1);
        }
        
        template<typename T>
            requires is_printable<T>
        static void print_as_string(const std::string header, const T& obj) {       
            std::unique_lock ul(m_logger_mutex);
            std::cout << header << obj << std::endl;
        }
        
        template<typename T>
            requires (not is_printable<T>) and is_elementwise_printable<T> and (not is_element_pairwise_printable<T>)
        static void print_as_string(const std::string header, const T& obj) {
            std::unique_lock ul(m_logger_mutex);
            std::cout << header << "{";
            bool first = true;
            for (const auto& c : obj) {
                if (!first) {
                    std::cout << ", ";
                }
                first = false;
                std::cout << static_cast<int>(c);
            }
            std::cout << "}" << std::endl;
        }
        
        template<typename T>
            requires (not is_printable<T>) and (not is_elementwise_printable<T>) and is_element_pairwise_printable<T>
        static void print_as_string(const std::string header, const T& obj) {
            std::unique_lock ul(m_logger_mutex);
            std::cout << header << "{";
            bool first = true;
            for (const auto& c : obj) {
                if (!first) {
                    std::cout << ", ";
                }
                first = false;
                std::cout << "(" << c.first << ", " << ")";
            }
            std::cout << "}" << std::endl;
        }
    };
}
