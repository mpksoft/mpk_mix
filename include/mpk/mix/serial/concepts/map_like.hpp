/** @file
 * @brief MapLike concept for associative-container types
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <concepts>

namespace mpk::mix {

template <typename T>
concept MapLike = requires(T const c, typename T::key_type const& key) {
    typename T::mapped_type;
    { c.find(key) } -> std::convertible_to<typename T::const_iterator>;
};

} // namespace mpk::mix
