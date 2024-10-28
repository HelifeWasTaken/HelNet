/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#undef SPDLOG_ACTIVE_LEVEL

#ifdef HL_NET_SPDLOG_ACTIVE_LEVEL
    #define SPDLOG_ACTIVE_LEVEL HL_NET_SPDLOG_ACTIVE_LEVEL
#else
    #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif

#include <spdlog/spdlog.h>

namespace hl
{
namespace net
{

namespace logger = spdlog;

#define HL_NET_LOG_LEVEL_TRACE    1
#define HL_NET_LOG_LEVEL_DEBUG    2
#define HL_NET_LOG_LEVEL_INFO     3
#define HL_NET_LOG_LEVEL_WARN     4
#define HL_NET_LOG_LEVEL_ERROR    5
#define HL_NET_LOG_LEVEL_CRITICAL 6
#define HL_NET_LOG_LEVEL_NONE     7

#define HL_NET_LOG_LEVEL_MIN      HL_NET_LOG_LEVEL_TRACE
#define HL_NET_LOG_LEVEL_MAX      HL_NET_LOG_LEVEL_NONE

#define HL_NET_LOG_TRACE(...)       SPDLOG_TRACE(__VA_ARGS__)
#define HL_NET_LOG_DEBUG(...)       SPDLOG_DEBUG(__VA_ARGS__)
#define HL_NET_LOG_INFO(...)        SPDLOG_INFO(__VA_ARGS__)
#define HL_NET_LOG_WARN(...)        SPDLOG_WARN(__VA_ARGS__)
#define HL_NET_LOG_ERROR(...)       SPDLOG_ERROR(__VA_ARGS__)
#define HL_NET_LOG_CRITICAL(...)    SPDLOG_CRITICAL(__VA_ARGS__)

#define HL_NET_LOG_LEVEL_CURRENT_ENUM   (hl::net::logger::level::level_enum::trace)

#if defined(HL_NET_LOG_LEVEL)
    #if ((HL_NET_LOG_LEVEL < HL_NET_LOG_LEVEL_MIN) || (HL_NET_LOG_LEVEL > HL_NET_LOG_LEVEL_MAX))
        #error "HL_NET_LOG_LEVEL must be between HL_NET_LOG_LEVEL_DEBUG and HL_NET_LOG_LEVEL_CRITICAL (Trace: 1, Debug: 2, Info: 3, Warn: 4, Error: 5, Critical: 6, None: 7)"
    #endif
#elif defined(DEBUG)
    #define HL_NET_LOG_LEVEL HL_NET_LOG_LEVEL_DEBUG
#else
    #define HL_NET_LOG_LEVEL HL_NET_LOG_LEVEL_INFO
#endif

#if HL_NET_LOG_LEVEL > HL_NET_LOG_LEVEL_TRACE
    #undef HL_NET_LOG_TRACE
    #define HL_NET_LOG_TRACE(...)
    #undef HL_NET_LOG_LEVEL_CURRENT_ENUM
    #define HL_NET_LOG_LEVEL_CURRENT_ENUM (hl::net::logger::level::level_enum::debug)
#endif

#if HL_NET_LOG_LEVEL > HL_NET_LOG_LEVEL_DEBUG
    #undef HL_NET_LOG_DEBUG
    #define HL_NET_LOG_DEBUG(...)
    #undef HL_NET_LOG_LEVEL_CURRENT_ENUM
    #define HL_NET_LOG_LEVEL_CURRENT_ENUM (hl::net::logger::level::level_enum::info)
#endif

#if HL_NET_LOG_LEVEL > HL_NET_LOG_LEVEL_INFO
    #undef HL_NET_LOG_INFO
    #define HL_NET_LOG_INFO(...)
    #undef HL_NET_LOG_LEVEL_CURRENT_ENUM
    #define HL_NET_LOG_LEVEL_CURRENT_ENUM (hl::net::logger::level::level_enum::warn)
#endif

#if HL_NET_LOG_LEVEL > HL_NET_LOG_LEVEL_WARN
    #undef HL_NET_LOG_WARN
    #define HL_NET_LOG_WARN(...)
    #undef HL_NET_LOG_LEVEL_CURRENT_ENUM
    #define HL_NET_LOG_LEVEL_CURRENT_ENUM (hl::net::logger::level::level_enum::error)
#endif

#if HL_NET_LOG_LEVEL > HL_NET_LOG_LEVEL_ERROR
    #undef HL_NET_LOG_ERROR
    #define HL_NET_LOG_ERROR(...)
    #undef HL_NET_LOG_LEVEL_CURRENT_ENUM
    #define HL_NET_LOG_LEVEL_CURRENT_ENUM (hl::net::logger::level::level_enum::critical)
#endif

#if HL_NET_LOG_LEVEL > HL_NET_LOG_LEVEL_CRITICAL
    #undef HL_NET_LOG_CRITICAL
    #define HL_NET_LOG_CRITICAL(...)
    #undef HL_NET_LOG_LEVEL_CURRENT_ENUM
    #define HL_NET_LOG_LEVEL_CURRENT_ENUM (hl::net::logger::level::level_enum::off)
#endif

#define HL_NET_SETUP_LOG_LEVEL() hl::net::logger::set_level(HL_NET_LOG_LEVEL_CURRENT_ENUM)

    namespace __internal {

        struct __hl_net_setup final {
            __hl_net_setup()
            {
                hl::net::logger::set_level(HL_NET_LOG_LEVEL_CURRENT_ENUM);
            }
        };

        #define HL_NET_SETUP_LOG_SERVICE() static inline const __hl_net_setup __hl_net_setup_service; // Create a global object so the constructor is called before main
    }

}
}
