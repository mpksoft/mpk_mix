/** @file
 * @brief throw_ — format-string-based exception helper.
 *
 * Copyright (C) 2025 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/detail/format.hpp"

#include <stdexcept>
#include <utility>

namespace mpk::mix {

template <typename Exception = std::runtime_error, typename... Args>
[[noreturn]]
auto throw_(MPKMIX_FORMAT_NS::format_string<Args...> fmt, Args&&... args) -> void
{
    throw Exception{mpk::mix::format(fmt, std::forward<Args>(args)...)};
}

template <typename Exception = std::runtime_error, typename... Args>
[[nodiscard]]
auto exception_ptr(MPKMIX_FORMAT_NS::format_string<Args...> fmt, Args&&... args)
    -> std::exception_ptr
{
    try
    {
        throw_<Exception>(fmt, std::forward<Args>(args)...);
    }
    catch (...)
    {
        return std::current_exception();
    }
}

} // namespace mpk::mix
