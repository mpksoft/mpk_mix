/** @file
 * @brief Optional-related utility functions and helpers
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <optional>

namespace mpk::mix
{

template <typename T>
static constexpr auto is_optional = false;

template <typename U>
static constexpr auto is_optional<std::optional<U>> = true;

template <typename T>
inline constexpr bool is_optional_v = is_optional<std::remove_cvref_t<T>>;

} // namespace mpk::mix
