/** @file
 * @brief Install terminate and fatal-signal handlers that print stack traces.
 *
 * Call install_crash_handlers() once, as early as possible in main(). When
 * MPKMIX_STACKTRACE is not defined, this is a no-op.
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once


namespace mpk::mix {

#ifdef MPKMIX_STACKTRACE

auto install_crash_handlers() -> void;

#else

inline auto install_crash_handlers() -> void {}

#endif

} // namespace mpk::mix
