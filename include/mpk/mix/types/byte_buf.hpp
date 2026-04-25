/** @file
 * @brief ByteBuf owning resizable byte buffer
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/types/byte_buf_fwd.hpp"
#include "mpk/mix/types/byte_span.hpp"
#include "mpk/mix/meta/enum_type.hpp"
#include "mpk/mix/meta/type.hpp"

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace mpk::mix
{

class ByteBuf
{
public:
    ByteBuf() = default;

    template <size_t Extent>
    ByteBuf(ConstByteSpan<Extent> data, size_t offset = 0)
    {
        load(data, offset);
    }

    auto clear() -> void
    {
        load(ConstByteSpan<0>{});
    }

    auto alloc(size_t size) -> ByteSpan<>
    {
        ensure_size(size);
        begin_ = 0;
        end_ = size;
        return mutable_view();
    }

    template <ByteOrConstByte B, size_t Extent>
    auto load(std::span<B, Extent> data, size_t offset = 0) -> void
    {
        ensure_size(offset + data.size());
        begin_ = offset;
        end_ = begin_ + data.size();
        memcpy(buf_.data() + begin_, data.data(), data.size());
    }

    template <size_t Extent>
    auto prepend(ConstByteSpan<Extent> prefix) -> void
    {
        if (begin_ < prefix.size())
            throw std::invalid_argument(
                "No space for prepending to byte buffer");
        begin_ -= prefix.size();
        memcpy(buf_.data() + begin_, prefix.data(), prefix.size());
    }

    template <std::integral T, std::convertible_to<T> X>
        requires(sizeof(T) >= sizeof(X))
    auto prepend(Type_Tag<T>, X x) -> void
    {
        auto t = T{x};
        prepend(const_span_of(t));
    }

    template <std::integral T, EnumType E>
        requires(
            std::is_convertible_v<std::underlying_type_t<E>, T>
            && sizeof(T) >= sizeof(E))
    auto prepend(Type_Tag<T>, E e) -> void
    {
        auto const& u = *reinterpret_cast<std::underlying_type_t<E> const*>(&e);
        auto t = T{u};
        prepend(const_span_of(t));
    }

    template <size_t Extent>
    auto append(ConstByteSpan<Extent> suffix) -> void
    {
        ensure_size(end_ + suffix.size());
        memcpy(buf_.data() + end_, suffix.data(), suffix.size());
        end_ += suffix.size();
    }

    template <size_t Extent>
    auto append(ByteSpan<Extent> suffix) -> void
    {
        append(ConstByteSpan<Extent>{suffix});
    }

    auto append(std::byte suffix) -> void
    {
        append(ConstByteSpan<1>{&suffix, 1});
    }

    template <std::integral T, std::convertible_to<T> X>
        requires(sizeof(T) >= sizeof(X))
    auto append(Type_Tag<T>, X x) -> void
    {
        auto t = T{x};
        append(const_span_of(t));
    }

    template <std::integral T, EnumType E>
        requires(
            std::is_convertible_v<std::underlying_type_t<E>, T>
            && sizeof(T) >= sizeof(E))
    auto append(Type_Tag<T>, E e) -> void
    {
        auto const& u = *reinterpret_cast<std::underlying_type_t<E> const*>(&e);
        auto t = T{u};
        append(const_span_of(t));
    }

    auto discard_prefix(size_t size) noexcept -> void
    {
        assert(size <= end_ - begin_);
        begin_ += size;
    }

    auto offset() const noexcept -> size_t
    {
        return begin_;
    }

    auto shrink() noexcept
    {
        if (begin_ == 0)
            return;

        if (end_ > begin_)
        {
            auto* d = buf_.data();
            memmove(d, d + begin_, end_ - begin_);
        }

        end_ -= begin_;
        begin_ = 0;
    }

    auto view() const -> ConstByteSpan<>
    {
        return {buf_.data() + begin_, end_ - begin_};
    }

    auto mutable_view() -> ByteSpan<>
    {
        return {buf_.data() + begin_, end_ - begin_};
    }

    auto size() const noexcept -> size_t
    {
        return end_ - begin_;
    }

    auto empty() const noexcept -> bool
    {
        return end_ == begin_;
    }

    auto data() noexcept -> std::byte*
    {
        return buf_.data() + begin_;
    }

    auto data() const noexcept -> std::byte const*
    {
        return buf_.data() + begin_;
    }

    operator ConstByteSpan<>() const noexcept
    {
        return view();
    }

    operator ByteSpan<>() noexcept
    {
        return mutable_view();
    }

private:
    auto ensure_size(size_t required_size) -> void
    {
        if (buf_.size() < required_size)
            buf_.resize(required_size);
    }

    template <typename T>
    auto const_span_of(T& t) -> ConstByteSpan<sizeof(T)>
    {
        auto const* byte_ptr = reinterpret_cast<std::byte const*>(&t);
        return ConstByteSpan<sizeof(T)>{byte_ptr, sizeof(T)};
    }

    std::vector<std::byte> buf_;
    size_t begin_{};
    size_t end_{};
};

} // namespace mpk::mix
