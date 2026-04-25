/** @file
 * @brief Zero_Tag sentinel representing a zero or empty value
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <concepts>

namespace mpk::mix
{

constexpr inline struct Zero_Tag final
{
} Zero;

template <std::default_initializable T>
auto operator==(Zero_Tag, T const& x) noexcept -> bool
{
    return x == T{};
}

template <std::default_initializable T>
auto operator==(T const& x, Zero_Tag) noexcept -> bool
{
    return x == T{};
}

} // namespace mpk::mix
