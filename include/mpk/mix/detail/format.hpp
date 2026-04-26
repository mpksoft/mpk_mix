/** @file
 * @brief Conditional polyfill: selects std::format (GCC ≥ 13) or fmtlib.
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#ifdef MPKMIX_USE_FMT_POLYFILL
#  include <fmt/format.h>
#  include <fmt/chrono.h>
#  define MPKMIX_FORMAT_NS fmt
#else
#  include <format>
#  define MPKMIX_FORMAT_NS std
#endif

#include <string>
#include <string_view>

namespace mpk::mix {

template <typename... Args>
auto format(
    MPKMIX_FORMAT_NS::format_string<Args...> fmt_str, Args&&... args)
    -> std::string
{
    return MPKMIX_FORMAT_NS::format(fmt_str, std::forward<Args>(args)...);
}

template <typename OutputIt, typename... Args>
auto format_to(
    OutputIt out,
    MPKMIX_FORMAT_NS::format_string<Args...> fmt_str, Args&&... args)
    -> OutputIt
{
    return MPKMIX_FORMAT_NS::format_to(out, fmt_str, std::forward<Args>(args)...);
}

inline auto vformat(
    std::string_view fmt_str,
    MPKMIX_FORMAT_NS::format_args args)
    -> std::string
{
    return MPKMIX_FORMAT_NS::vformat(fmt_str, args);
}

template <typename... Args>
auto make_format_args(Args&&... args)
{
    return MPKMIX_FORMAT_NS::make_format_args(std::forward<Args>(args)...);
}

} // namespace mpk::mix
