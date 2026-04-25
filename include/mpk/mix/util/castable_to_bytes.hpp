/** @file
 * @brief CastableToBytes concept for types safely reinterpretable as byte arrays
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <type_traits>

namespace mpk::mix
{

template <typename T>
concept CastableToBytes = std::is_trivially_copyable_v<T>
                          && std::is_nothrow_default_constructible_v<T>
                          && std::is_trivially_destructible_v<T>;

} // namespace mpk::mix
