/** @file
 * @brief EnumType concept for scoped and unscoped enumerations
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <type_traits>

namespace mpk::mix
{

template <typename T>
concept EnumType = std::is_enum_v<T>;

} // namespace mpk::mix
