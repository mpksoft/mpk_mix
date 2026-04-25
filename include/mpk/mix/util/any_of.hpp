/** @file
 * @brief any_of() variadic helper: true if any argument satisfies a predicate
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <concepts>

namespace mpk::mix
{

template <typename T, typename... Ts>
concept AnyOf = (std::same_as<T, Ts> || ...);

} // namespace mpk::mix
