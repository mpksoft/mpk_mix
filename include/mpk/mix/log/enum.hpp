/** @file
 * @brief Quill codec for logging enum values as their name strings
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/log/defs.hpp"
#include "mpk/mix/meta/enum_type.hpp"

#include <magic_enum/magic_enum.hpp>

#if MPKMIX_LOG_ENGINE == MPKMIX_LOG_QUILL

#include "quill/DeferredFormatCodec.h"

template <mpk::mix::EnumType E>
struct fmtquill::formatter<E>
{
    constexpr auto parse(fmtquill::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    auto format(E const& e, fmtquill::format_context& ctx) const
        -> decltype(ctx.out())
    {
        auto name = magic_enum::enum_name(e);
        return fmtquill::format_to(
            ctx.out(), "{}", fmtquill::string_view{name.data(), name.size()});
    }
};

// Codec<EnumType> not needed: the primary quill::Codec handles enums natively.

#endif
