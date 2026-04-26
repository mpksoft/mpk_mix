/** @file
 * @brief Dynamic (runtime) format string helper.
 *
 * Copyright (C) 2025 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/detail/format.hpp"

#include <string>
#include <string_view>

namespace mpk::mix {

/**
 * @brief Format with a runtime format string (not a compile-time literal).
 *
 * Unlike std::format, the format string is evaluated at runtime via vformat.
 * Useful when the format string is constructed dynamically.
 */
template <typename... Args>
auto dynamic_format(std::string_view rt_fmt_str, Args&&... args) -> std::string
{
    return mpk::mix::vformat(rt_fmt_str, mpk::mix::make_format_args(args...));
}

} // namespace mpk::mix
