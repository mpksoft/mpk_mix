/** @file
 * @brief Umbrella header for the logging abstraction
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/log/level.hpp"

#if MPKMIX_LOG_ENGINE == MPKMIX_LOG_NONE

#include <filesystem>
#include <string>
#include <string_view>

namespace mpk::mix {

struct Logger final
{
};

inline auto start_logging() -> void
{
}

inline auto stop_logging() -> void
{
}

inline auto set_log_level(Logger*, LogLevel) -> void
{
}

inline auto set_log_level(LogLevel) -> void
{
}

inline auto default_logger() -> Logger*
{
    return nullptr;
}

inline auto file_logger(
    std::filesystem::path const& /*file_name*/,
    std::string const& /*logger_name*/) -> Logger*
{
    return nullptr;
}

inline auto get_logger(std::string_view /*logger_name*/) -> Logger*
{
    return nullptr;
}

} // namespace mpk::mix

#define MPKMIX_LOG_TRACE_L3(...) static_assert(true)
#define MPKMIX_LOG_TRACE_L2(...) static_assert(true)
#define MPKMIX_LOG_TRACE_L1(...) static_assert(true)
#define MPKMIX_LOG_DEBUG(...) static_assert(true)
#define MPKMIX_LOG_INFO(...) static_assert(true)
#define MPKMIX_LOG_WARNING(...) static_assert(true)
#define MPKMIX_LOG_ERROR(...) static_assert(true)
#define MPKMIX_LOG_CRITICAL(...) static_assert(true)
#define MPKMIX_FLUSH_LOGGER(logger) static_assert(true)

#elif MPKMIX_LOG_ENGINE == MPKMIX_LOG_QUILL

#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include "quill/sinks/ConsoleSink.h"
#include "quill/sinks/FileSink.h"
#include "quill/std/Chrono.h"

#include <filesystem>
#include <string>
#include <string_view>

namespace mpk::mix {

using quill::Logger;

inline auto start_logging() -> void
{
    quill::BackendOptions backend_options;
    quill::Backend::start(backend_options);
}

inline auto stop_logging() -> void
{
    quill::Backend::stop();
}

inline auto default_logger() -> Logger*
{
    auto console_sink =
        quill::Frontend::create_or_get_sink<quill::ConsoleSink>("sink_id_1");
    return quill::Frontend::create_or_get_logger("root", std::move(console_sink));
}

inline auto file_logger(
    std::filesystem::path const& file_name,
    std::string const& logger_name) -> Logger*
{
    auto file_sink = quill::Frontend::create_or_get_sink<quill::FileSink>(
        file_name,
        []()
        {
            quill::FileSinkConfig cfg;
            cfg.set_open_mode('w');
            return cfg;
        }(),
        quill::FileEventNotifier{});
    return quill::Frontend::create_or_get_logger(logger_name, std::move(file_sink));
}

inline auto get_logger(std::string_view logger_name) -> Logger*
{
    return quill::Frontend::get_logger(std::string{logger_name});
}

inline auto set_log_level(Logger* logger, LogLevel log_level) -> void
{
    logger->set_log_level(log_level);
}

inline auto set_log_level(LogLevel log_level) -> void
{
    set_log_level(default_logger(), log_level);
}

} // namespace mpk::mix

#define MPKMIX_LOG_CHECKED(logger, LEVEL, ...)  \
    do                                          \
    {                                           \
        if ((logger) != nullptr)                \
        {                                       \
            LOG_##LEVEL((logger), __VA_ARGS__); \
        }                                       \
    } while (0)

#define MPKMIX_LOG_TRACE_L3(logger, ...) \
    MPKMIX_LOG_CHECKED(logger, TRACE_L3, __VA_ARGS__)

#define MPKMIX_LOG_TRACE_L2(logger, ...) \
    MPKMIX_LOG_CHECKED(logger, TRACE_L2, __VA_ARGS__)

#define MPKMIX_LOG_TRACE_L1(logger, ...) \
    MPKMIX_LOG_CHECKED(logger, TRACE_L1, __VA_ARGS__)

#define MPKMIX_LOG_DEBUG(logger, ...) \
    MPKMIX_LOG_CHECKED(logger, DEBUG, __VA_ARGS__)

#define MPKMIX_LOG_INFO(logger, ...) \
    MPKMIX_LOG_CHECKED(logger, INFO, __VA_ARGS__)

#define MPKMIX_LOG_WARNING(logger, ...) \
    MPKMIX_LOG_CHECKED(logger, WARNING, __VA_ARGS__)

#define MPKMIX_LOG_ERROR(logger, ...) \
    MPKMIX_LOG_CHECKED(logger, ERROR, __VA_ARGS__)

#define MPKMIX_LOG_CRITICAL(logger, ...) \
    MPKMIX_LOG_CHECKED(logger, CRITICAL, __VA_ARGS__)

#define MPKMIX_FLUSH_LOGGER(logger)  \
    do                               \
    {                                \
        if ((logger) != nullptr)     \
        {                            \
            (logger)->flush_log();   \
        }                            \
    } while (0)

#elif MPKMIX_LOG_ENGINE == MPKMIX_LOG_CERR

#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <syncstream>
#include <vector>

namespace mpk::mix {

struct Logger final
{
};

namespace detail {

inline LogLevel log_level = LogLevel::Info;

inline auto level_label(LogLevel level) -> std::string_view
{
    switch (level)
    {
    case LogLevel::TraceL3: return "TRACE_L3";
    case LogLevel::TraceL2: return "TRACE_L2";
    case LogLevel::TraceL1: return "TRACE_L1";
    case LogLevel::Debug: return "DEBUG";
    case LogLevel::Info: return "INFO";
    case LogLevel::Warning: return "WARNING";
    case LogLevel::Error: return "ERROR";
    case LogLevel::Critical: return "CRITICAL";
    case LogLevel::None: return "NONE";
    }
    __builtin_unreachable();
}

template <typename T>
auto format_arg(T&& arg) -> std::string
{
    std::ostringstream oss;
    oss << std::forward<T>(arg);
    return oss.str();
}

template <typename... Args>
auto format_each(Args&&... args) -> std::vector<std::string>
{
    return {format_arg(std::forward<Args>(args))...};
}

inline auto log_to_cerr(
    LogLevel level,
    const char* file,
    int line,
    std::string_view fmt,
    std::vector<std::string> formatted_args) -> void
{
    if (level < log_level)
        return;

    auto ostr = std::osyncstream{std::cerr};
    ostr << level_label(level) << ": " << file << ":" << line << ": ";

    size_t pos = 0;
    for (size_t iarg = 0, nargs = formatted_args.size(); iarg < nargs; ++iarg)
    {
        const auto& arg = formatted_args[iarg];
        auto arg_pos = fmt.find("{}", pos);
        if (arg_pos == std::string_view::npos)
        {
            if (!(iarg + 1 == nargs && arg.empty()))
            {
                ostr << "...\nTOO FEW PLACEHOLDERS IN LOG FORMAT STRING\n";
                ostr.emit();
                std::terminate();
            }
            ostr << fmt.substr(pos) << '\n';
            return;
        }
        ostr << fmt.substr(pos, arg_pos - pos) << arg;
        pos = arg_pos + 2;
    }
    auto arg_pos = fmt.find("{}", pos);
    if (arg_pos != std::string_view::npos)
    {
        ostr << "...\nTOO MANY PLACEHOLDERS IN LOG FORMAT STRING\n";
        ostr.emit();
        std::terminate();
    }
    ostr << fmt.substr(pos) << '\n';
}

} // namespace detail

inline auto start_logging() -> void
{
}

inline auto stop_logging() -> void
{
}

inline auto set_log_level(Logger* /*logger*/, LogLevel level) -> void
{
    detail::log_level = level;
}

inline auto set_log_level(LogLevel level) -> void
{
    detail::log_level = level;
}

inline auto default_logger() -> Logger*
{
    static Logger logger;
    return &logger;
}

inline auto file_logger(
    std::filesystem::path const& /*file_name*/,
    std::string const& /*logger_name*/) -> Logger*
{
    return default_logger();
}

inline auto get_logger(std::string_view /*logger_name*/) -> Logger*
{
    return default_logger();
}

} // namespace mpk::mix

#define MPKMIX_LOG_TO_CERR(LEVEL, logger, FORMAT_STRING, ...)       \
    ::mpk::mix::detail::log_to_cerr(                                \
        ::mpk::mix::LogLevel::LEVEL,                                \
        __FILE__,                                                   \
        __LINE__,                                                   \
        FORMAT_STRING,                                              \
        ::mpk::mix::detail::format_each(__VA_ARGS__))

#define MPKMIX_LOG_TRACE_L3(logger, ...) static_assert(true)
#define MPKMIX_LOG_TRACE_L2(logger, ...) static_assert(true)
#define MPKMIX_LOG_TRACE_L1(logger, ...) static_assert(true)
#define MPKMIX_LOG_DEBUG(logger, ...) MPKMIX_LOG_TO_CERR(Debug, logger, ##__VA_ARGS__)
#define MPKMIX_LOG_INFO(logger, ...) MPKMIX_LOG_TO_CERR(Info, logger, ##__VA_ARGS__)
#define MPKMIX_LOG_WARNING(logger, ...) MPKMIX_LOG_TO_CERR(Warning, logger, ##__VA_ARGS__)
#define MPKMIX_LOG_ERROR(logger, ...) MPKMIX_LOG_TO_CERR(Error, logger, ##__VA_ARGS__)
#define MPKMIX_LOG_CRITICAL(logger, ...) MPKMIX_LOG_TO_CERR(Critical, logger, ##__VA_ARGS__)
#define MPKMIX_FLUSH_LOGGER(logger) static_assert(true)

#else
#error "Unknown MPKMIX_LOG_ENGINE value"
#endif

// Convenience macros that log to the default logger.
#define MPKMIX_LOG_DEFAULT_DEBUG(...) \
    MPKMIX_LOG_DEBUG(::mpk::mix::default_logger(), ##__VA_ARGS__)
#define MPKMIX_LOG_DEFAULT_INFO(...) \
    MPKMIX_LOG_INFO(::mpk::mix::default_logger(), ##__VA_ARGS__)
#define MPKMIX_LOG_DEFAULT_WARNING(...) \
    MPKMIX_LOG_WARNING(::mpk::mix::default_logger(), ##__VA_ARGS__)
#define MPKMIX_LOG_DEFAULT_ERROR(...) \
    MPKMIX_LOG_ERROR(::mpk::mix::default_logger(), ##__VA_ARGS__)
#define MPKMIX_LOG_DEFAULT_CRITICAL(...) \
    MPKMIX_LOG_CRITICAL(::mpk::mix::default_logger(), ##__VA_ARGS__)
