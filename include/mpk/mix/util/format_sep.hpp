/** @file
 * @brief Format a variadic list of values with a separator between them.
 *
 * Copyright (C) 2025 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <sstream>
#include <string>
#include <string_view>

namespace mpk::mix {

/**
 * @brief Stream args to a string with sep inserted between each pair.
 *
 * Example: format_sep(", ", 1, "hello", 3.14) → "1, hello, 3.14"
 */
template <typename... Args>
auto format_sep(std::string_view sep, Args&&... args) -> std::string
{
    std::ostringstream s;
    auto first = true;
    ([&]<typename Arg>(Arg&& arg) {
        if (!first)
            s << sep;
        s << std::forward<Arg>(arg);
        first = false;
    }(std::forward<Args>(args)),
    ...);
    return s.str();
}

} // namespace mpk::mix
