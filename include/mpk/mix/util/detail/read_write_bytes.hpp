/** @file
 * @brief Low-level byte-level read/write helpers
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/meta/type.hpp"
#include "mpk/mix/types/byte_span.hpp"
#include "mpk/mix/util/throw.hpp"

#include <cstring>

namespace mpk::mix::detail
{

using ByteSpanDyn = ByteSpan<>;
using ConstByteSpanDyn = ConstByteSpan<>;

template <typename T>
using ByteSpanFor = ByteSpan<sizeof(T)>;

template <typename T>
using ConstByteSpanFor = ConstByteSpan<sizeof(T)>;

template <typename T, ByteOrConstByte B>
auto byte_span_for(B* ptr, Type_Tag<T> = {})
{
    return std::span<B, sizeof(T)>{ptr, sizeof(T)};
}

template <ByteOrConstByte B, size_t extent>
auto write_bytes(ByteSpan<extent> dst, std::span<B, extent> src) -> void
{
    if constexpr (extent == std::dynamic_extent)
    {
        if (dst.size() != src.size())
            throw_(
                "write_bytes: buffer size mismatch: src size={}, dst size={}",
                src.size(),
                dst.size());
    }
    memcpy(dst.data(), src.data(), src.size());
}

template <size_t extent, typename T>
auto write_typed(ByteSpan<extent> dst, T const& value) -> void
{
    write_bytes(dst, byte_span(value));
}

template <ByteOrConstByte B, size_t extent, typename T>
auto read_typed(std::span<B, extent> src, T& value) -> void
{
    write_bytes(byte_span(value), src);
}

} // namespace mpk::mix::detail
