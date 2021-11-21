#include "signal_interrupt.h"
#include "signal.h"

util::signal_interrupt::signal_interrupt(std::function<void()> call) {
    m_global_call = new std::function(call);
    
    struct sigaction t;
    t.sa_handler = handle;
    
    sigaction(SIGINT, &t, nullptr);
}
        
util::signal_interrupt::~signal_interrupt() {
    struct sigaction t;
    t.sa_handler = SIG_DFL;
    
    sigaction(SIGINT, &t, nullptr);
    
    delete m_global_call;
    m_global_call = nullptr;
}        
        
void util::signal_interrupt::handle(int) {
    (*m_global_call)();
}
        
std::function<void()>* util::signal_interrupt::m_global_call;
