/** @file
 * @brief StructType concept and field-accessor traits
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/meta/type.hpp"

namespace mpk::mix
{

template <typename T>
concept StructType = requires(T t, T const ct) {
    fields_of(t);
    fields_of(ct);
    tuple_tag_of(Type<T>);
    field_names_of(Type<T>);
};

} // namespace mpk::mix
