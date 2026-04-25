/** @file
 * @brief Quill codec registration for Strong<Traits> log formatting
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/log/defs.hpp"
#include "mpk/mix/strong/fwd.hpp"

#if MPKMIX_LOG_ENGINE == MPKMIX_LOG_QUILL

#include "quill/DeferredFormatCodec.h"

template <mpk::mix::StrongType T>
struct fmtquill::formatter<T>
{
    constexpr auto parse(fmtquill::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(T const& c, fmtquill::format_context& ctx) const
        -> decltype(ctx.out())
    {
        return fmtquill::format_to(ctx.out(), "{}", c.v);
    }
};

template <mpk::mix::StrongType T>
struct quill::Codec<T> : quill::DeferredFormatCodec<T>
{
};

#endif
