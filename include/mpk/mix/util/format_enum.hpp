/** @file
 * @brief Format enum values as their magic_enum name strings
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/meta/enum_type.hpp"

#include <magic_enum/magic_enum.hpp>

#include <format>


namespace mpk::mix {

template <EnumType E>
struct EnumFormatter
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(E const& value, FormatContext& ctx) const
    {
        return std::format_to(ctx.out(), "{}", magic_enum::enum_name(value));
    }
};

} // namespace mpk::mix


template <mpk::mix::EnumType E>
struct std::formatter<E> final : mpk::mix::EnumFormatter<E>
{};
