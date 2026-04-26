/** @file
 * @brief std::formatter base for types that have operator<<.
 *
 * Copyright (C) 2025 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/detail/format.hpp"

#include <concepts>
#include <ostream>
#include <sstream>

namespace mpk::mix {

/** @brief Concept: T is formattable via operator<<(ostream&, T). */
template <typename T>
concept OstreamFormattable = requires(std::ostream& os, const T& t)
{
    { os << t } -> std::same_as<std::ostream&>;
};

/**
 * @brief Base struct for std::formatter specializations that delegate to operator<<.
 *
 * Usage:
 * @code
 * template <> struct std::formatter<MyType> : mpk::mix::OstreamFormatter<MyType> {};
 * @endcode
 * Or use the MPKMIX_DECL_OSTREAM_FORMATTER macro (must be used at global scope).
 */
template <typename T>
struct OstreamFormatter
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx)
    { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const T& value, FormatContext& ctx) const
    {
        std::ostringstream s;
        s << value;
        return MPKMIX_FORMAT_NS::format_to(ctx.out(), "{}", s.str());
    }
};

} // namespace mpk::mix

// Declare std::formatter<T> delegating to operator<<.
// Must be used at global scope, outside any namespace.
#define MPKMIX_DECL_OSTREAM_FORMATTER(T)                                    \
    template <>                                                             \
    struct MPKMIX_FORMAT_NS::formatter<T> final                             \
        : ::mpk::mix::OstreamFormatter<T>                                   \
    {}
