/** @file
 * @brief Compile-time hash value derived from a type's name
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/meta/type.hpp"
#include "mpk/mix/strong/strong.hpp"

#include <typeinfo>


namespace mpk::mix {

MPKMIX_STRONG_TYPE(TypeHash, size_t);

template <typename T>
inline auto type_hash(Type_Tag<T> = {}) noexcept -> TypeHash
{
    return TypeHash{typeid(T).hash_code()};
}

} // namespace mpk::mix
