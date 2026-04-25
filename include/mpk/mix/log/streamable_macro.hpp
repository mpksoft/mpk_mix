/** @file
 * @brief MPKMIX_LOG_STREAMABLE macro: full quill codec+formatter for ostream-able types
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/log/defs.hpp"
#include "mpk/mix/log/format_streamable_macro.hpp"
#include "mpk/mix/log/trivial_codec_macro.hpp"

#if MPKMIX_LOG_ENGINE == MPKMIX_LOG_NONE || MPKMIX_LOG_ENGINE == MPKMIX_LOG_CERR

#define MPKMIX_LOG_STREAMABLE(T) static_assert(true)

#elif MPKMIX_LOG_ENGINE == MPKMIX_LOG_QUILL

/** MPKMIX_LOG_STREAMABLE(T) registers both formatter and Codec for T.
 *  T must implement operator<<(std::ostream&, const T&).
 *  If T contains a comma, use a type alias instead.
 *  Expand in the global namespace. */
#define MPKMIX_LOG_STREAMABLE(T)     \
    MPKMIX_LOG_FORMAT_STREAMABLE(T); \
    MPKMIX_LOG_TRIVIAL_CODEC(T)

#endif
