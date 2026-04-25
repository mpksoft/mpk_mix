/** @file
 * @brief Range that yields consecutive index-like values
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/meta/type.hpp"
#include "mpk/mix/meta/zero.hpp"
#include "mpk/mix/strong/fwd.hpp"
#include "mpk/mix/util/any_of.hpp"
#include "mpk/mix/util/detail/index_like.hpp"

#include <compare>
#include <iterator>

namespace mpk::mix
{

namespace detail
{

template <IndexLikeType T>
struct IndexTraits final
{
    using index_type = T;
    using value_type = T const;
    using pointer = T const*;
    using reference = T const;
};

template <IndexLikeType T>
class IndexIterator final
{
public:
    using Traits = IndexTraits<T>;

    using iterator_category = std::random_access_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using index_type = typename Traits::index_type;
    using value_type = typename Traits::value_type;
    using pointer = typename Traits::pointer;
    using reference = typename Traits::reference;

    IndexIterator() noexcept = default;

    explicit IndexIterator(index_type v) noexcept : v_{v}
    {
    }

    auto operator*() const noexcept -> reference
    {
        return v_;
    }

    auto operator->() const noexcept -> pointer
    {
        return &v_;
    }

    auto operator++() noexcept -> IndexIterator&
    {
        ++v_;
        return *this;
    }

    auto operator--() noexcept -> IndexIterator&
    {
        --v_;
        return *this;
    }

    auto operator++(int) noexcept -> IndexIterator
    {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    auto operator--(int) noexcept -> IndexIterator
    {
        auto tmp = *this;
        --(*this);
        return tmp;
    }

    auto operator+=(difference_type d) noexcept -> IndexIterator&
    {
        v_ += d;
        return *this;
    }

    auto operator-=(difference_type d) noexcept -> IndexIterator&
    {
        v_ -= d;
        return *this;
    }

    friend auto operator+(IndexIterator it, difference_type d) noexcept
        -> IndexIterator
    {
        return IndexIterator{*it + d};
    }

    friend auto operator-(IndexIterator it, difference_type d) noexcept
        -> IndexIterator
    {
        return IndexIterator{*it - d};
    }

    friend auto operator-(IndexIterator lhs, IndexIterator rhs) noexcept
        -> difference_type
    {
        return static_cast<difference_type>(*lhs)
               - static_cast<difference_type>(*rhs);
    }

    auto operator[](difference_type d) -> reference
    {
        return v_ + d;
    }

    friend auto operator<=>(
        IndexIterator const& a, IndexIterator const& b) noexcept
        -> std::strong_ordering = default;

private:
    index_type v_;
};

} // namespace detail

template <detail::IndexLikeType T>
class IndexRange final
{
public:
    using Index = T;
    using Count = detail::IndexDiffType<T>;
    using const_iterator = detail::IndexIterator<T>;
    using iterator = detail::IndexIterator<T>;

    explicit IndexRange(Count size) noexcept :
      IndexRange{Index{0}, Index{0} + size}
    {
    }

    template <AnyOf<T, Zero_Tag> Ib, AnyOf<T, Zero_Tag> Ie>
    static auto range(Ib begin, Ie end) noexcept -> IndexRange
    {
        auto b = to<Index>(begin);
        auto e = to<Index>(end);
        return {b, e};
    }

    template <AnyOf<T, Zero_Tag> I, AnyOf<Count, Zero_Tag> C>
    static auto sized_range(I begin, C size) noexcept -> IndexRange
    {
        auto b = to<Index>(begin);
        auto s = to<Count>(size);
        return {b, b + s};
    }

    auto begin() const noexcept -> const_iterator
    {
        return const_iterator{begin_};
    }

    auto end() const noexcept -> const_iterator
    {
        return const_iterator{end_};
    }

    auto size() const noexcept -> Count
    {
        return end_ - begin_;
    }

    auto empty() const noexcept -> bool
    {
        return end_ == begin_;
    }

    auto contains(T index) const noexcept -> bool
    {
        return begin_ <= index && index < end_;
    }

    auto operator==(IndexRange const&) const noexcept -> bool = default;

protected:
    IndexRange(T begin, T end) : begin_{begin}, end_{end}
    {
    }

    template <typename To>
    static auto to(To x) noexcept -> To
    {
        return x;
    }

    template <typename To>
    static auto to(Zero_Tag) noexcept -> To
    {
        return To{0};
    }

    T begin_;
    T end_;
};

template <std::integral T>
auto index_range(T size) -> IndexRange<T>
{
    return IndexRange<T>::sized_range(Zero, size);
}

template <StrongIndexType T>
auto index_range(Type_Tag<T>, detail::IndexDiffType<T> size) -> IndexRange<T>
{
    return IndexRange<T>::sized_range(Zero, size);
}

template <StrongIndexType T>
auto index_range(detail::IndexDiffType<T> size) -> IndexRange<T>
{
    return index_range(Type<T>, size);
}

template <detail::IndexLikeType T>
auto index_range(T begin, std::type_identity_t<T> end) -> IndexRange<T>
{
    return IndexRange<T>::range(begin, end);
}

template <StrongIndexType T>
auto index_range(T begin, detail::IndexDiffType<T> size) -> IndexRange<T>
{
    return IndexRange<T>::sized_range(begin, size);
}

template <detail::IndexLikeType T>
auto sized_index_range(T begin, detail::IndexDiffType<T> size) -> IndexRange<T>
{
    return IndexRange<T>::sized_range(begin, size);
}

} // namespace mpk::mix
