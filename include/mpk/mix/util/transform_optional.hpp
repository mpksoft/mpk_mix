/** @file
 * @brief transform_optional() applies a function to an optional's value if present
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/meta/type.hpp"

#include <functional>
#include <optional>

namespace mpk::mix
{

template <typename To, typename From>
auto transform_optional(
    std::optional<From> const& value, auto transform, Type_Tag<To> = {})
    -> std::optional<To>
{
    if (!value)
        return std::nullopt;
    return transform(*value);
}

template <typename To, typename From>
auto transform_optional(
    std::optional<From> const& value, Type_Tag<To> to = {})
    -> std::optional<To>
{
    return transform_optional(value, std::identity{}, to);
}

} // namespace mpk::mix
