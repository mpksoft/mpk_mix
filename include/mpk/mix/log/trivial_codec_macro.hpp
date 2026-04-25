/** @file
 * @brief MPKMIX_LOG_TRIVIAL_CODEC macro for registering trivially-formattable quill codecs
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/log/defs.hpp"

#if MPKMIX_LOG_ENGINE == MPKMIX_LOG_NONE || MPKMIX_LOG_ENGINE == MPKMIX_LOG_CERR

#define MPKMIX_LOG_TRIVIAL_CODEC(T) static_assert(true)

#elif MPKMIX_LOG_ENGINE == MPKMIX_LOG_QUILL

#include "quill/DeferredFormatCodec.h"

/** MPKMIX_LOG_TRIVIAL_CODEC(T) specializes quill::Codec<T> via DeferredFormatCodec.
 *  Requires fmtquill::formatter<T> to be defined separately.
 *  Expand in the global namespace. */
#define MPKMIX_LOG_TRIVIAL_CODEC(T)                         \
    template <>                                             \
    struct quill::Codec<T> : quill::DeferredFormatCodec<T>  \
    {                                                       \
    }

#endif
