/** @file
 * @brief StrongSpan<S,I> span indexed by a strong type
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/strong/fwd.hpp"
#include "mpk/mix/util/index_range.hpp"

#include <array>
#include <cassert>
#include <limits>
#include <span>

namespace mpk::mix
{

template <typename V, StrongIndexType I, std::size_t Extent = std::dynamic_extent>
struct StrongSpan
{
    using Index = I;
    using Count = typename I::StrongDiff;
    using value_type = V;
    using Weak = std::span<V, Extent>;
    using iterator = typename Weak::iterator;
    using reverse_iterator = typename Weak::reverse_iterator;
    using DynamicStrongSpan = StrongSpan<V, I, std::dynamic_extent>;

    Weak v;

    StrongSpan() = default;

    template <typename It>
        requires(Extent == std::dynamic_extent)
    explicit StrongSpan(It first, Count size) : v{first, size.v}
    {
    }

    template <typename It, typename End>
        requires(Extent == std::dynamic_extent)
    explicit StrongSpan(It first, End end) : v{first, end}
    {
    }

    template <std::size_t N>
        requires(Extent == std::dynamic_extent || Extent == N)
    constexpr StrongSpan(std::type_identity_t<V> (&arr)[N]) noexcept : v{arr}
    {
    }

    template <typename U, std::size_t N>
        requires(Extent == std::dynamic_extent || Extent == N)
    constexpr StrongSpan(std::array<U, N>& arr) noexcept : v{arr}
    {
    }

    template <typename U, std::size_t N>
        requires(Extent == std::dynamic_extent || Extent == N)
    constexpr StrongSpan(std::array<U, N> const& arr) noexcept : v{arr}
    {
    }

    template <class R>
        requires(Extent == std::dynamic_extent)
    constexpr StrongSpan(R&& range) : v(std::forward<R>(range))
    {
    }

    explicit StrongSpan(Weak v) : v(v)
    {
    }

    auto operator[](I i) noexcept -> V&
    {
        return v[i.v];
    }
    auto operator[](I i) const noexcept -> V const&
    {
        return v[i.v];
    }

    auto at(I i) -> V&
    {
        return v.at(i.v);
    }
    auto at(I i) const -> V const&
    {
        return v.at(i.v);
    }

    auto begin() const noexcept -> iterator
    {
        return v.begin();
    }
    auto end() const noexcept -> iterator
    {
        return v.end();
    }

    auto rbegin() const noexcept -> iterator
    {
        return v.rbegin();
    }
    auto rend() const noexcept -> iterator
    {
        return v.rend();
    }

    auto front() -> V&
    {
        return v.front();
    }
    auto back() -> V&
    {
        return v.back();
    }
    auto front() const -> V const&
    {
        return v.front();
    }
    auto back() const -> V const&
    {
        return v.back();
    }

    auto data() noexcept -> V*
    {
        return v.data();
    }
    auto data() const noexcept -> V const*
    {
        return v.data();
    }

    auto empty() const noexcept -> bool
    {
        return v.empty();
    }
    auto size() const noexcept -> Count
    {
        return checked_count_(v.size());
    }

    constexpr auto subspan(Index offset) const -> DynamicStrongSpan
    {
        return DynamicStrongSpan{v.subspan(offset.v)};
    }

    constexpr auto subspan(Count size) const -> DynamicStrongSpan
    {
        return DynamicStrongSpan{v.subspan(0, size.v)};
    }

    constexpr auto subspan(Index offset, Count count) const -> DynamicStrongSpan
    {
        return DynamicStrongSpan{v.subspan(offset.v, count.v)};
    }

    constexpr auto subspan(Index begin_offset, Index end_offset) const
        -> DynamicStrongSpan
    {
        return DynamicStrongSpan{
            v.subspan(begin_offset.v, (end_offset - begin_offset).v)};
    }

    constexpr auto index_range() const noexcept -> IndexRange<I>
    {
        return IndexRange<I>(size());
    }

    auto operator==(StrongSpan const& that) const noexcept -> bool
    {
        return v == that.v;
    }

    auto operator<=>(StrongSpan const& that) const noexcept
    {
        return v <=> that.v;
    }

    static auto checked_count_(Weak::size_type c) noexcept -> Count
    {
        assert(c <= std::numeric_limits<typename Count::Weak>::max());
        return Count(c);
    }
};

} // namespace mpk::mix
