/** @file
 * @brief Canonical 'type' and 'value' field name constants for serialization
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <string_view>

namespace mpk::mix::serial::detail {

constexpr inline auto type_field_name = std::string_view{"type"};
constexpr inline auto value_field_name = std::string_view{"value"};

} // namespace mpk::mix::serial::detail
