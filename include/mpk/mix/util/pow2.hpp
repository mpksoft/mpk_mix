/** @file
 * @brief Power-of-two floor and ceiling for unsigned integral types.
 *
 * Copyright (C) 2024 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/meta/type.hpp"

#include <concepts>
#include <type_traits>


namespace mpk::mix {

template <std::unsigned_integral T>
constexpr auto floor2(Type_Tag<T>, std::type_identity_t<T> x)
    -> T
{
    auto d = T{1};
    while (x & ~d)
    {
        x &= ~d;
        d <<= 1;
    }
    return x;
}

template <std::unsigned_integral T>
constexpr auto ceil2(Type_Tag<T> tag, std::type_identity_t<T> x)
    -> T
{
    auto xf = floor2(tag, x);
    return x == xf ? x : xf << 1;
}

template <std::unsigned_integral T>
constexpr auto floor2(T x)
    -> T
{ return floor2(Type<T>, x); }

template <std::unsigned_integral T>
constexpr auto ceil2(T x)
    -> T
{ return ceil2(Type<T>, x); }

} // namespace mpk::mix
