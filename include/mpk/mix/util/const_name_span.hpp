/** @file
 * @brief ConstNameSpan alias and const_name_span() factory for static string-view spans
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/util/const_span.hpp"

#include <string_view>


namespace mpk::mix {

using ConstNameSpan = std::span<const std::string_view>;

template <typename... Ids, std::same_as<std::string_view>... Args>
auto const_name_span(TypePack_Tag<Ids...> ids, Args... values)
    -> ConstNameSpan
{ return const_span(ids, Type<std::string_view>, values...); }

} // namespace mpk::mix
