/** @file
 * @brief Const_Tag, Indirect_Tag, and Unsafe_Tag for FuncRef customization
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/meta/const.hpp"

namespace mpk::mix
{

// Indirect_Tag / Unsafe_Tag stay here; Const_Tag / Const moved to meta/const.hpp.

constexpr inline struct Indirect_Tag final
{
} Indirect;

// ---

constexpr inline struct Unsafe_Tag final
{
} Unsafe;

} // namespace mpk::mix
