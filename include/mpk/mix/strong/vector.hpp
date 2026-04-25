/** @file
 * @brief StrongVector<V,I> vector indexed by a strong type
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/strong/fwd.hpp"
#include "mpk/mix/util/index_range.hpp"

#include <cassert>
#include <limits>
#include <vector>

namespace mpk::mix
{

template <typename V, StrongIndexType I>
struct StrongVector
{
    using Index = I;
    using Count = typename I::StrongDiff;
    using value_type = V;
    using Weak = std::vector<V>;
    using reference = typename Weak::reference;
    using const_reference = typename Weak::const_reference;
    using iterator = typename Weak::iterator;
    using const_iterator = typename Weak::const_iterator;
    using reverse_iterator = typename Weak::reverse_iterator;
    using const_reverse_iterator = typename Weak::const_reverse_iterator;

    Weak v;

    StrongVector() = default;

    explicit StrongVector(Count size) : v(size.v)
    {
    }

    explicit StrongVector(Count size, value_type const& init) : v(size.v, init)
    {
    }

    explicit StrongVector(Weak v) : v(std::move(v))
    {
    }

    StrongVector(std::initializer_list<V> init) : v{init}
    {
    }

    template <class InputIt>
    StrongVector(InputIt first, InputIt last) : v(first, last)
    {
    }

    // TODO: More constructors

    auto operator[](I i) noexcept -> reference
    {
        return v[i.v];
    }
    auto operator[](I i) const noexcept -> const_reference
    {
        return v[i.v];
    }

    auto at(I i) -> reference
    {
        return v.at(i.v);
    }
    auto at(I i) const -> const_reference
    {
        return v.at(i.v);
    }

    auto begin() noexcept -> iterator
    {
        return v.begin();
    }
    auto end() noexcept -> iterator
    {
        return v.end();
    }
    auto begin() const noexcept -> const_iterator
    {
        return v.begin();
    }
    auto end() const noexcept -> const_iterator
    {
        return v.end();
    }
    auto cbegin() const noexcept -> const_iterator
    {
        return v.begin();
    }
    auto cend() const noexcept -> const_iterator
    {
        return v.end();
    }

    auto rbegin() noexcept -> iterator
    {
        return v.rbegin();
    }
    auto rend() noexcept -> iterator
    {
        return v.rend();
    }
    auto rbegin() const noexcept -> const_iterator
    {
        return v.rbegin();
    }
    auto rend() const noexcept -> const_iterator
    {
        return v.rend();
    }
    auto crbegin() const noexcept -> const_iterator
    {
        return v.crbegin();
    }
    auto crend() const noexcept -> const_iterator
    {
        return v.crend();
    }

    auto front() -> reference
    {
        return v.front();
    }
    auto back() -> reference
    {
        return v.back();
    }
    auto front() const -> const_reference
    {
        return v.front();
    }
    auto back() const -> const_reference
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
    auto capacity() const noexcept -> Count
    {
        return checked_count_(v.capacity());
    }
    auto reserve(Count c) -> void
    {
        v.reserve(c.v);
    }

    auto clear() noexcept -> void
    {
        v.clear();
    }
    // TODO: insert
    auto push_back(V const& element) -> void
    {
        v.push_back(element);
    }
    auto push_back(V&& element) -> void
    {
        v.push_back(std::move(element));
    }

    template <typename... Args>
    auto emplace_back(Args&&... args) -> V&
    {
        return v.emplace_back(std::forward<Args>(args)...);
    }

    auto pop_back() -> void
    {
        v.pop_back();
    }

    auto resize(Count c) -> void
    {
        v.resize(c.v);
    }
    auto resize(Count c, V const& init) -> void
    {
        v.resize(c.v, init);
    }
    auto swap(StrongVector& that) -> void
    {
        v.swap(that.v);
    }

    constexpr auto index_range() const noexcept -> IndexRange<I>
    {
        return IndexRange<I>(size());
    }

    auto operator==(StrongVector const& that) const noexcept -> bool
    {
        return v == that.v;
    }

    auto operator<=>(StrongVector const& that) const noexcept
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
