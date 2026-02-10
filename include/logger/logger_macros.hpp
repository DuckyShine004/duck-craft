#pragma once

#include <fmt/format.h>

#include "logger/logger.hpp"

#define LOG__CALL(SEV, ...)                                                                                                                                                                            \
    do {                                                                                                                                                                                               \
        logger::Logger::get_instance().log(logger::Severity::SEV, __FILE__, __func__, __LINE__, fmt::format(__VA_ARGS__));                                                                             \
    } while (0)

// clang-format off
#ifdef NDEBUG
    #define LOG_DEBUG(...) ((void) 0)
#else
    #define LOG_DEBUG(...) LOG__CALL(DEBUG, __VA_ARGS__)
#endif
// clang-format on

#define LOG_INFO(...) LOG__CALL(INFO, __VA_ARGS__)

#define LOG_WARN(...) LOG__CALL(WARN, __VA_ARGS__)

#define LOG_ERROR(...) LOG__CALL(ERROR, __VA_ARGS__)

#define LOG_FATAL(...) LOG__CALL(FATAL, __VA_ARGS__)
