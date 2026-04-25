/** @file
 * @brief IndexLike concept for types usable as array indices
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/strong/fwd.hpp"

#include <concepts>
#include <utility>

namespace mpk::mix::detail
{

template <typename T>
concept IndexLikeType = std::integral<T> || StrongIndexType<T>;

template <IndexLikeType T>
using IndexDiffType = decltype(std::declval<T>() - std::declval<T>());

} // namespace mpk::mix::detail
