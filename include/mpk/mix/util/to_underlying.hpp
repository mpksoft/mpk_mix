/** @file
 * @brief to_underlying() converts an enum to its underlying integer type
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/strong/fwd.hpp"
#include "mpk/mix/types/fixed_length_string.hpp"

#include <utility>

namespace mpk::mix
{

template <typename T>
    requires std::is_arithmetic_v<T>
constexpr auto to_underlying(T x) noexcept -> T
{
    return x;
}

constexpr auto to_underlying(char const* x) noexcept -> char const*
{
    return x;
}

constexpr inline auto to_underlying(std::string_view x) noexcept
    -> std::string_view
{
    return x;
}

constexpr inline auto to_underlying(std::string x) noexcept -> std::string
{
    return x;
}

template <typename T>
    requires std::is_enum_v<T>
constexpr auto to_underlying(T x) noexcept -> std::underlying_type_t<T>
{
    return static_cast<std::underlying_type_t<T>>(x);
}

template <detail::fixed_length_string_type T>
constexpr auto to_underlying(T x) -> std::string
{
    return std::string{x.view()};
}

template <StrongType T>
constexpr auto to_underlying(T x)
{
    return to_underlying(std::move(x.v));
}

template <typename T>
concept ConvertibleToUnderlyingType = requires(T x) { to_underlying(x); };

} // namespace mpk::mix
