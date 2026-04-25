/** @file
 * @brief Dense bitset-backed set of index-like values
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/util/detail/index_like.hpp"
#include "mpk/mix/util/index_range.hpp"
#include "mpk/mix/util/pow2.hpp"
#include "mpk/mix/strong/strong.hpp"
#include "mpk/mix/func_ref/tags.hpp"
#include "mpk/mix/meta/zero.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>


namespace mpk::mix {

namespace detail {
template <size_t> struct uint_of_size;

template <size_t size>
using uint_of_size_t = uint_of_size<size>::type;

template <> struct uint_of_size<8> final { using type = uint8_t; };
template <> struct uint_of_size<16> final { using type = uint16_t; };
template <> struct uint_of_size<32> final { using type = uint32_t; };
template <> struct uint_of_size<64> final { using type = uint64_t; };

} // namespace detail

template <detail::IndexLikeType Index_, size_t count_ = 8*sizeof(Index_)>
class IndexSet
{
public:
    using Index = Index_;
    using Count = detail::IndexDiffType<Index>;
    static constexpr auto raw_count = count_;
    static constexpr auto count = Count{count_};

    using Storage = detail::uint_of_size_t<mpk::mix::ceil2(raw_count)>;

    class const_iterator final
    {
    public:
        using Traits = detail::IndexTraits<Index>;

        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using index_type        = typename Traits::index_type;
        using value_type        = typename Traits::value_type;
        using pointer           = typename Traits::pointer;
        using reference         = typename Traits::reference;

        const_iterator() noexcept = default;

        explicit const_iterator(Storage data, index_type v) noexcept :
            data_{data},
            v_{v}
        {}

        auto operator*() const noexcept
            -> reference
        { return v_; }

        auto operator->() const noexcept
            -> pointer
        { return &v_; }

        auto operator++() noexcept -> const_iterator&
        {
            do
                ++v_;
            while (raw(v_) < raw(count) && !at_element());
            return *this;
        }

        auto operator--() noexcept -> const_iterator&
        {
            assert(v_ != mpk::mix::Zero);
            do
                --v_;
            while (v_ != mpk::mix::Zero && !at_element());
            assert(at_element());
            return *this;
        }

        auto operator++(int) noexcept -> const_iterator
        { auto tmp = *this; ++(*this); return tmp; }

        auto operator--(int) noexcept -> const_iterator
        { auto tmp = *this; --(*this); return tmp; }

        friend auto operator<=>(const const_iterator& a,
                                const const_iterator& b) noexcept
            -> std::strong_ordering
        {
            assert (a.data_ == b.data_);
            return a.v_ <=> b.v_;
        }

        friend auto operator==(const const_iterator& a,
                               const const_iterator& b) noexcept
            -> bool
        {
            assert (a.data_ == b.data_);
            return a.v_ == b.v_;
        }

    private:
        auto at_element() const noexcept
            -> bool
        {
            assert(raw(v_) < raw(count));
            return (data_ & (one_ << raw(v_))) != 0;
        }

        Storage data_{};
        index_type v_;
    };

    using iterator = const_iterator;


    constexpr IndexSet() noexcept = default;

    constexpr explicit IndexSet(Count n) :
        data_( (Storage{1u} << raw(n)) - 1u )
    { assert(n <= count); }

    /* implicit */ IndexSet(std::initializer_list<Index> indices) noexcept
    { set(indices); }

    auto set(std::initializer_list<Index> indices) noexcept
        -> void
    {
        for (auto index : indices)
            set(index);
    }

    auto set(Index index) noexcept
        -> void
    {
        assert(raw(index) < raw(count));
        data_ |= one_ << raw(index);
    }

    auto clear(std::initializer_list<Index> indices) noexcept
        -> void
    {
        for (auto index : indices)
            clear(index);
    }

    auto clear(Index index) noexcept
        -> void
    {
        assert(raw(index) < raw(count));
        data_ &= ~(one_ << raw(index));
    }

    auto toggle(std::initializer_list<Index> indices) noexcept
        -> void
    {
        for (auto index : indices)
            toggle(index);
    }

    auto toggle(Index index) noexcept
        -> void
    {
        assert(raw(index) < raw(count));
        data_ ^= one_ << raw(index);
    }

    constexpr auto contains(Index index) const noexcept
        -> bool
    { return raw(index) < raw(count) && (data_ & (one_ << raw(index))) != 0; }

    constexpr auto empty() const noexcept
        -> bool
    { return data_ == 0; }

    static constexpr auto all() noexcept
        -> IndexSet
    { return { Unsafe, (one_ << raw(count)) - one_ }; }

    constexpr auto size() const noexcept
        -> size_t
    {
        auto result = size_t{};
        for (auto d=data_; d!=0; d>>=1)
            result += d & one_;
        return result;
    }

    auto begin() const noexcept
        -> iterator
    {
        auto result = iterator{ data_, Index{0} };
        if (!contains(Index{0}))
            ++result;
        return result;
    }

    auto end() const noexcept
        -> iterator
    { return iterator{ data_, count+Index{0} }; }


    auto operator|(const IndexSet& that) const noexcept -> IndexSet
    { return { Unsafe, static_cast<Storage>(data_ | that.data_) }; }

    auto operator&(const IndexSet& that) const noexcept -> IndexSet
    { return { Unsafe, static_cast<Storage>(data_ & that.data_) }; }

    auto operator^(const IndexSet& that) const noexcept -> IndexSet
    { return { Unsafe, static_cast<Storage>(data_ ^ that.data_) }; }

    auto operator|=(const IndexSet& that) noexcept -> IndexSet&
    { data_ |= that.data_;   return *this; }

    auto operator&=(const IndexSet& that) noexcept -> IndexSet&
    { data_ &= that.data_;   return *this; }

    auto operator^=(const IndexSet& that) noexcept -> IndexSet&
    { data_ ^= that.data_;   return *this; }

    auto operator~() const noexcept -> IndexSet
    { return *this ^ all(); }

    constexpr auto data() const noexcept
    { return data_; }

private:
    IndexSet(Unsafe_Tag, Storage data):
        data_{ data }
    {}

    static constexpr auto one_ = Storage{1};

    Storage data_{};
};

} // namespace mpk::mix
