/** @file
 * @brief Parse a scalar string into a typed Value
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/value/type_fwd.hpp"
#include "mpk/mix/value/value_fwd.hpp"

#include <string_view>

namespace mpk::mix::value {

auto parse_simple_value(std::string_view text, const Type* type) -> Value;

} // namespace mpk::mix::value
