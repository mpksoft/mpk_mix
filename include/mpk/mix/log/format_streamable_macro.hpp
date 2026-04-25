/** @file
 * @brief MPKMIX_LOG_FORMAT_STREAMABLE macro for ostream-based quill formatters
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/log/defs.hpp"

#if MPKMIX_LOG_ENGINE == MPKMIX_LOG_NONE

#define MPKMIX_LOG_FORMAT_STREAMABLE(T) static_assert(true)

#elif MPKMIX_LOG_ENGINE == MPKMIX_LOG_QUILL

#include <quill/bundled/fmt/ostream.h>

/** MPKMIX_LOG_FORMAT_STREAMABLE(T) registers an fmtquill formatter for T
 *  that delegates to operator<<(std::ostream&, const T&).
 *  Expand in the global namespace. */
#define MPKMIX_LOG_FORMAT_STREAMABLE(T)                          \
    template <>                                                  \
    struct fmtquill::formatter<T> : fmtquill::ostream_formatter \
    {                                                            \
    }

#elif MPKMIX_LOG_ENGINE == MPKMIX_LOG_CERR

#define MPKMIX_LOG_FORMAT_STREAMABLE(T) static_assert(true)

#endif
