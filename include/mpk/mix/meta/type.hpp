/** @file
 * @brief Type_Tag<T> empty tag and Type<T> variable template for type passing
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

namespace mpk::mix
{

template <typename T>
struct Type_Tag final
{
    using type = T;
};

template <typename T>
constexpr inline auto Type = Type_Tag<T>{};

template <typename T>
constexpr inline auto is_type_tag_type = false;

template <typename T>
constexpr inline auto is_type_tag_type<Type_Tag<T>> = true;

#if __cplusplus >= 202002L
template <typename T>
concept TypeTagType = is_type_tag_type<T>;

template <typename T>
concept NonTypeTagType = !is_type_tag_type<T>;
#endif

} // namespace mpk::mix
