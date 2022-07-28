#include "logger.h"

bool util::logger::m_enable_log_status;
bool util::logger::m_enable_log_warning;
bool util::logger::m_enable_log_error;
bool util::logger::m_enable_log_debug;
std::mutex util::logger::m_logger_mutex;
