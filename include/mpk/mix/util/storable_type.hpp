/** @file
 * @brief StorableType concept for types safe to store as raw bytes
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/util/castable_to_bytes.hpp"

#include <span>

namespace mpk::mix
{

template <typename T>
concept PlainStorableType = CastableToBytes<T>;

namespace detail
{

template <typename>
constexpr inline auto is_static_extent_span_of_plain_storable_v = false;

template <PlainStorableType T, size_t extent>
    requires(extent != std::dynamic_extent)
constexpr inline auto
    is_static_extent_span_of_plain_storable_v<std::span<T, extent>> = true;

template <typename>
constexpr inline auto is_dynamic_extent_span_v = false;

template <typename T>
constexpr inline auto is_dynamic_extent_span_v<std::span<T>> = true;

} // namespace detail

template <typename T>
concept StaticExtentSpanOfPlainStorableType =
    detail::is_static_extent_span_of_plain_storable_v<T>;

template <typename T>
concept DynamicExtentSpanType = detail::is_dynamic_extent_span_v<T>;

template <typename T>
concept StorableType =
    (PlainStorableType<T> || StaticExtentSpanOfPlainStorableType<T>)
    && !DynamicExtentSpanType<T>;

namespace detail
{

template <StorableType T>
struct StorableTypeTraits;

template <StorableType T>
    requires PlainStorableType<T>
struct StorableTypeTraits<T> final
{
    using write_arg_t = T const&;
    using read_arg_t = T&;
    static constexpr auto data_size = sizeof(T);
};

template <StorableType T>
    requires StaticExtentSpanOfPlainStorableType<T>
struct StorableTypeTraits<T> final
{
    using write_arg_t = std::span<typename T::element_type const>;
    using read_arg_t = T;
    static constexpr auto data_size = T::extent;
};

} // namespace detail

} // namespace mpk::mix
