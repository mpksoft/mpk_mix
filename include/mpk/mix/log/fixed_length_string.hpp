/** @file
 * @brief Quill codec registration for FixedLengthString log formatting
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/log/defs.hpp"
#include "mpk/mix/types/fixed_length_string.hpp"

#if MPKMIX_LOG_ENGINE == MPKMIX_LOG_QUILL

#include "quill/DeferredFormatCodec.h"

template <typename CharT, size_t N, typename Traits>
struct fmtquill::formatter<mpk::mix::BasicFixedLengthString<CharT, N, Traits>>
{
    using this_string = mpk::mix::BasicFixedLengthString<CharT, N, Traits>;

    constexpr auto parse(fmtquill::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(this_string const& c, fmtquill::format_context& ctx) const
        -> decltype(ctx.out())
    {
        auto sv = c.view();
        return fmtquill::format_to(
            ctx.out(), "{}", fmtquill::string_view{sv.data(), sv.size()});
    }
};

template <typename CharT, size_t N, typename Traits>
struct quill::Codec<mpk::mix::BasicFixedLengthString<CharT, N, Traits>>
    : quill::DeferredFormatCodec<mpk::mix::BasicFixedLengthString<CharT, N, Traits>>
{
};

#endif
