#pragma once

#include <cstring>
#include <string>
#include <vector>
#include <stdint.h>

#include "memory_region.h"

namespace network {

class pkt_buffer {
public:
    pkt_buffer() {}
    pkt_buffer(const size_t capacity) : m_buffer(capacity) {}
    
    memory_region writeable_region() {
        return memory_region(m_buffer.data() + m_valid_length, m_buffer.size() - m_valid_length);
    }
    
    memory_region readable_region() {
        return memory_region(m_buffer.data(), m_valid_length);
    }
        
    bool write(const memory_region& region) {
        if (writeable_region().contains(region)) {
            m_valid_length += region.size();
            return true;
        }
        
        return false;
    }
    
    bool read(const memory_region& region) {        
        if (readable_region().contains(region)) {
            if (m_valid_length == region.size()) {
                m_valid_length = 0;
            } else {                            
                memory_region dest;
                dest.use(m_buffer);
                dest.splice(0, m_valid_length - region.size()).move(dest.splice(region.size(), m_valid_length - region.size()));
                                
                m_valid_length -= region.size();
            }
            return true;
        }
        
        return false;       
    }
    
    bool contains_readable(const memory_region& local_region, const memory_region& compare_region) {
        return readable_region().contains(local_region) && local_region.compare(compare_region);
    }
private:
    std::vector<uint8_t> m_buffer;
    size_t m_valid_length = 0;
};
}
