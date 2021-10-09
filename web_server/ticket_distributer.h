#pragma once

#include <unordered_map>
#include <stdint.h>

namespace web_server {
    template<typename num_T, typename ref_T>
    class ticket_distributer {
    public:
        num_T get_ticket(ref_T* ref) {
            assert(ref != nullptr);
            auto current = m_current_ticket++;
                        
            ref->on_destroy([current, this]() {
                m_tickets.erase(current);
            });
            
            m_tickets[current] = ref;
            
            return current;
        }
    private:
        std::unordered_map<num_T, ref_T*> m_tickets;
        num_T m_current_ticket = 0;
    };
}
