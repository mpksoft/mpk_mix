/** @file
 * @brief Canonical type-name string for use in serialized type tags
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/meta/type.hpp"

#include <string_view>

#define MPKMIX_DECL_TYPE_NAME(T, name)                   \
    [[maybe_unused]] constexpr inline auto type_name(    \
        ::mpk::mix::Type_Tag<T>) -> std::string_view     \
    {                                                    \
        return name;                                     \
    }                                                    \
    static_assert(true)
