/** @file
 * @brief Exception-capturing wrappers (logging-free base).
 *
 * Catches any exception thrown by a callable and returns it as
 * std::exception_ptr rather than propagating it.  No logging is performed —
 * callers (or compatibility shims) are responsible for logging if desired.
 *
 * Copyright (C) 2024 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <concepts>
#include <exception>
#include <tuple>


namespace mpk::mix {

/// Non-void overload: returns {exception_ptr, R{}}.
template <typename F, typename... Args>
auto with_exceptions(F const& f, Args&&... args)
    -> std::tuple<std::exception_ptr, decltype(f(std::forward<Args>(args)...))>
    requires (!std::same_as<decltype(f(std::forward<Args>(args)...)), void>)
{
    using R = decltype(f(std::forward<Args>(args)...));
    try
    {
        return { nullptr, f(std::forward<Args>(args)...) };
    }
    catch (...)
    {
        return { std::current_exception(), R{} };
    }
}

/// Void overload: returns exception_ptr (nullptr on success).
template <typename F, typename... Args>
auto with_exceptions(F const& f, Args&&... args)
    -> std::exception_ptr
    requires std::same_as<decltype(f(std::forward<Args>(args)...)), void>
{
    try
    {
        f(std::forward<Args>(args)...);
        return nullptr;
    }
    catch (...)
    {
        return std::current_exception();
    }
}

} // namespace mpk::mix
