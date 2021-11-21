#pragma once

#include <functional>

namespace util {
    class signal_interrupt {
    public:
        signal_interrupt(std::function<void()> call);        
        ~signal_interrupt();   
        
        signal_interrupt(signal_interrupt&) = delete;
        signal_interrupt(signal_interrupt&&) = delete;
        void operator=(signal_interrupt&) = delete;
        void operator=(signal_interrupt&&) = delete;
    private:
        static void handle(int);
        static std::function<void()>* m_global_call;
    };
}
