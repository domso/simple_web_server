#pragma once

#include <cstring>
#include <type_traits>
#include <optional>
#include <charconv>

#include <stdint.h>
#include "memory_region_view.h"

namespace network {
    class memory_region {
    public:
        memory_region() : m_data(nullptr), m_size(0) {}
        memory_region(uint8_t* data, const size_t size) : m_data(data), m_size(size) {}

        template<typename T>
        memory_region(T& container) {
            m_data = reinterpret_cast<uint8_t*>(container.data());
            m_size = container.size() * sizeof(decltype(*container.data()));
        }    

        template<typename T>
        void use(T& container) {
            m_data = reinterpret_cast<uint8_t*>(container.data());
            m_size = container.size() * sizeof(decltype(*container.data()));
        }    
        
        template<typename T>
        void use(T* data, const size_t size) {
            m_data = reinterpret_cast<const uint8_t*>(data);
            m_size = size;
        }    
        
        uint8_t* data() {
            return m_data;
        }
        
        const uint8_t* data() const {
            return m_data;
        }
        
        size_t size() const {
            return m_size;
        }
        
        template<typename T>
        std::optional<const T*> cast_to() const {
            static_assert(std::is_trivially_copyable<T>::value);
            if (m_size == sizeof(T)) {
                return reinterpret_cast<T*>(m_data);
            }
            
            return std::nullopt;
        }
        
        template<typename T>
        std::optional<T*> cast_to() {
            static_assert(std::is_trivially_copyable<T>::value);
            if (m_size == sizeof(T)) {
                return reinterpret_cast<T*>(m_data);
            }
            
            return std::nullopt;
        }    

        template<typename T>
        std::optional<T> convert_to_int() const {
            T result;
            auto [ptr, ec] = std::from_chars(reinterpret_cast<char*>(m_data), reinterpret_cast<char*>(m_data) + m_size, result);
            if (ec == std::errc()) {
                return result;
            } else {
                return std::nullopt;
            }
        }
        
        template<typename T>
        typename std::enable_if<std::is_constructible<T, uint8_t*, size_t>::value, T>::type export_to() {
            return T(m_data, m_size);
        }

        template<typename T>
        typename std::enable_if<std::is_constructible<T, char*, size_t>::value, T>::type export_to() {
            return T(reinterpret_cast<char*>(m_data), m_size);
        }

        template<typename T>
        typename std::enable_if<std::is_constructible<T, uint8_t*, size_t>::value, const T>::type export_to() const {
            return T(m_data, m_size);
        }

        template<typename T>
        typename std::enable_if<std::is_constructible<T, char*, size_t>::value, const T>::type export_to() const {
            return T(reinterpret_cast<char*>(m_data), m_size);
        }
        
        memory_region splice(const size_t pos, const size_t n) const {            
            if (pos + n <= m_size) {
                return memory_region(m_data + pos, n);
            }       
            
            if (pos < m_size) {
                return memory_region(m_data + pos, m_size - pos);
            }
            
            return memory_region(nullptr, 0);
        }
        
        memory_region offset_front(const size_t offset) const {
            return splice(offset, m_size - offset);
        }
            
        memory_region offset_back(const size_t offset) const {
            return splice(0, m_size - offset);
        }
        
        bool contains(const memory_region_view& region) const {
            return m_data <= region.m_data && (region.m_data + region.m_size) <= (m_data + m_size);
        }
        
        bool contains(const void* ptr) const {
            return m_data <= ptr && ptr < m_data + m_size;
        }
        
        bool compare(const memory_region_view& region) const {
            if (m_size == region.size()) {
                return std::memcmp(region.data(), m_data, m_size) == 0;
            }            
            
            return false;
        }
        
        bool overlap(const memory_region_view& region) const {
            return contains(reinterpret_cast<const void*>(region.data())) || 
                   contains(reinterpret_cast<const void*>(region.data() + region.size() - 1)) ||
                   region.contains(reinterpret_cast<const void*>(data())) ||
                   region.contains(reinterpret_cast<const void*>(data() + size()));
        }
        
        void move(const memory_region& region) {
            std::memmove(m_data, region.m_data, std::min(m_size, region.size()));
        }
        
        template<typename T>
        void push_back_into(T& container) const {
            for (size_t i = 0; i < m_size; i++) {
                container.push_back(m_data[i]);
            }
        }
        
        class iterator {
        public:
            iterator() : m_data(nullptr) {};
            iterator(uint8_t* data) : m_data(data) {};
            
            bool operator==(const iterator& it) const {return m_data == it.m_data;}
            bool operator!=(const iterator& it) const {return m_data != it.m_data;}
            bool operator<(const iterator& it) const {return m_data < it.m_data;}
            bool operator>(const iterator& it) const {return m_data > it.m_data;}
            bool operator<=(const iterator& it) const {return m_data <= it.m_data;}
            bool operator>=(const iterator& it) const {return m_data >= it.m_data;}
            iterator operator++(int) {return iterator(m_data++);}
            iterator operator--(int) {return iterator(m_data--);}
            iterator& operator++() {m_data++; return *this;}
            iterator& operator--() {m_data--; return *this;}      
            
            uint8_t& operator*() {return *m_data;}
        private:
            uint8_t* m_data;
        };
        
        class const_iterator {
        public:
            const_iterator() : m_data(nullptr) {};
            const_iterator(const uint8_t* data) : m_data(data) {};
            
            bool operator==(const const_iterator& it) const {return m_data == it.m_data;}
            bool operator!=(const const_iterator& it) const {return m_data != it.m_data;}
            bool operator<(const const_iterator& it) const {return m_data < it.m_data;}
            bool operator>(const const_iterator& it) const {return m_data > it.m_data;}
            bool operator<=(const const_iterator& it) const {return m_data <= it.m_data;}
            bool operator>=(const const_iterator& it) const {return m_data >= it.m_data;}
            const_iterator operator++(int) {return const_iterator(m_data++);}
            const_iterator operator--(int) {return const_iterator(m_data--);}
            const_iterator& operator++() {m_data++; return *this;}
            const_iterator& operator--() {m_data--; return *this;}      
            
            const uint8_t& operator*() {return *m_data;}
        private:
            const uint8_t* m_data;
        };
        
        iterator begin() {
            return iterator(m_data);
        }
        
        iterator rbegin() {
            return iterator(m_data + m_size - 1);
        }
        
        iterator end() {
            return iterator(m_data + m_size);
        }
        
        iterator rend() {
            return iterator(m_data - 1);
        }   
        
        const_iterator begin() const {
            return const_iterator(m_data);
        }
        
        const_iterator rbegin() const {
            return const_iterator(m_data + m_size - 1);
        }
        
        const_iterator end() const {
            return const_iterator(m_data + m_size);
        }
        
        const_iterator rend() const {
            return const_iterator(m_data - 1);
        }         
    private:
        uint8_t* m_data;
        size_t m_size;
    };
}
