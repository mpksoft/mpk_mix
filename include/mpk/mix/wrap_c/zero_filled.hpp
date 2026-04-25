/** @file
 * @brief ZeroFilled<T> value-initialises a struct to all-zero bytes
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/meta/type.hpp"

#include <cstring>

namespace mpk::mix::wrap_c {

template <typename T>
auto zero_filled(Type_Tag<T> = {}) -> T
{
    T result;
    memset(&result, 0, sizeof(T));
    return result;
}

} // namespace mpk::mix::wrap_c
