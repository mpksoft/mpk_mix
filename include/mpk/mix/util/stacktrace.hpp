/** @file
 * @brief Crash-time stack trace capture and formatting.
 *
 * When MPKMIX_STACKTRACE is defined, this module captures a stack trace
 * at every C++ exception throw site (via __cxa_throw interception) into a
 * thread-local slot, and provides helpers to format the most-recent throw
 * stack and to format the current stack on demand.
 *
 * When MPKMIX_STACKTRACE is not defined, all helpers are no-ops returning
 * empty strings — so consumer code can include this header unconditionally.
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <cstddef>
#include <string>


namespace mpk::mix {

#ifdef MPKMIX_STACKTRACE

auto current_stacktrace_string(std::size_t skip = 0) -> std::string;

auto last_throw_stacktrace_string() -> std::string;

auto clear_last_throw_stacktrace() -> void;

#else // !MPKMIX_STACKTRACE

inline auto current_stacktrace_string(std::size_t = 0) -> std::string
{ return {}; }

inline auto last_throw_stacktrace_string() -> std::string
{ return {}; }

inline auto clear_last_throw_stacktrace() -> void {}

#endif // MPKMIX_STACKTRACE

} // namespace mpk::mix
