/** @file
 * @brief Create a static-storage const std::span from a compile-time argument list
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/meta/type.hpp"
#include "mpk/mix/meta/type_pack.hpp"

#include <concepts>
#include <span>


namespace mpk::mix {

template <typename... Ids, typename T, std::same_as<T>... Args>
auto const_span(TypePack_Tag<Ids...>, Type_Tag<T>, Args... values)
    -> std::span<const T>
{
    static const std::array<T, sizeof...(Args)> result = { values... };
    return result;
}

} // namespace mpk::mix
