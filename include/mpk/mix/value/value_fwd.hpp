/** @file
 * @brief Value forward declaration and span aliases.
 *
 * Copyright (C) 2024-2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <span>

namespace mpk::mix::value {

class Value;

using ValueSpan = std::span<Value>;
using ConstValueSpan = std::span<const Value>;

} // namespace mpk::mix::value
