/** @file
 * @brief Binomial coefficient for integral types.
 *
 * Copyright (C) 2024 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/meta/type.hpp"

#include <cassert>
#include <type_traits>
#include <utility>
#include <vector>


namespace mpk::mix {

template <typename T>
    requires std::is_integral_v<T>
auto binomial(Type_Tag<T>,
              std::type_identity_t<T> n,
              std::type_identity_t<T> k)
    -> T
{
    if constexpr (std::is_signed_v<T>)
    {
        assert(k >= 0);
        assert(n >= 0);
    }
    assert(k <= n);
    auto v1 = std::vector<T>(n+1);
    auto v2 = std::vector<T>(n+1);
    for (T i=0; i<=n; ++i)
    {
        v2[0] = 1;
        for (T j=1; j<i; ++j)
            v2[j] = v1[j-1] + v1[j];
        v2[i] = 1;
        std::swap(v1, v2);
    }
    return v1[k];
}

} // namespace mpk::mix
