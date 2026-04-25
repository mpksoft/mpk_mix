/** @file
 * @brief maybe_const<T,IsConst> selects T const& or T& at compile time
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <concepts>
#include <type_traits>

namespace mpk::mix
{

template <typename T, typename U>
concept MaybeConst = std::same_as<T, U> || std::same_as<T, const U>;

template <typename T, typename U>
using AsConstAs = std::conditional_t<
    std::is_const_v<T>,
    const std::remove_const_t<U>,
    std::remove_const_t<U>>;

} // namespace mpk::mix
