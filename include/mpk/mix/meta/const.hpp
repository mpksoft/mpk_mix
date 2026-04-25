/** @file
 * @brief Const_Tag<V> compile-time constant wrapper for non-type template arguments
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

namespace mpk::mix
{

template <auto value_>
struct Const_Tag final
{
    using type = decltype(value_);
    static constexpr auto value = value_;
};

template <auto value_>
constexpr inline auto Const = Const_Tag<value_>{};

} // namespace mpk::mix
