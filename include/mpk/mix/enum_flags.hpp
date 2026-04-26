/** @file
 * @brief EnumFlags<E> — a type-safe bitset for enum values.
 *
 * Copyright (C) 2025-2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/meta/enum_type.hpp"
#include "mpk/mix/util/format_seq.hpp"
#include "mpk/mix/util/format_streamable.hpp"

#include <magic_enum/magic_enum.hpp>

#include <bit>
#include <bitset>
#include <cassert>
#include <concepts>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <type_traits>

namespace mpk::mix {

template <EnumType E>
class EnumFlags final
{
public:
    using key_type = E;
    using value_type = E;

    static constexpr auto capacity = magic_enum::enum_count<E>();

    EnumFlags() = default;

    template <std::same_as<E>... Ts>
    constexpr /* implicit */ EnumFlags(E flag0, Ts... flags)
    {
        set(flag0, flags...);
    }

    static auto full() -> EnumFlags
    {
        auto result = EnumFlags{};
        result.storage_.set();
        return result;
    }

    constexpr auto set(E flag, bool value) -> void
    {
        storage_.set(index(flag), value);
    }

    template <std::same_as<E>... Ts>
    constexpr auto set(E flag0, Ts... flags) -> void
    {
        set(flag0, true);
        (set(flags, true), ...);
    }

    constexpr auto clear() -> void
    {
        storage_.reset();
    }

    template <std::same_as<E>... Ts>
    constexpr auto clear(E flag0, Ts... flags) -> void
    {
        set(flag0, false);
        (set(flags, false), ...);
    }

    auto operator&=(const EnumFlags& that) -> EnumFlags&
    {
        storage_ &= that.storage_;
        return *this;
    }

    auto operator|=(const EnumFlags& that) -> EnumFlags&
    {
        storage_ |= that.storage_;
        return *this;
    }

    auto operator^=(const EnumFlags& that) -> EnumFlags&
    {
        storage_ ^= that.storage_;
        return *this;
    }

    auto operator~() const noexcept
        -> EnumFlags<E>
    {
        auto result = *this;
        result.storage_.flip();
        return result;
    }

    auto operator==(const EnumFlags& that) const noexcept -> bool = default;

    auto all() const noexcept -> bool
    {
        return storage_.all();
    }

    auto any() const noexcept -> bool
    {
        return storage_.any();
    }

    constexpr auto size() const noexcept -> size_t
    {
        return std::popcount(storage_.to_ullong());
    }

    auto contains(E flag) const -> bool
    {
        return storage_.test(index(flag));
    }

    auto empty() const noexcept -> bool
    {
        return !any();
    }

    operator bool() const noexcept
    {
        return any();
    }

    friend auto operator<<(std::ostream& s, const EnumFlags& f) -> std::ostream&
    {
        auto r = std::ranges::transform_view{
            f,
            [](E e){ return magic_enum::enum_name(e); }};
        return s << '{' << format_seq(r, ", ") << '}';
    }

private:
    static auto index(E flag) -> size_t
    {
        auto opt_index = magic_enum::enum_index(flag);
        if (!opt_index.has_value())
            throw std::invalid_argument("Specified enum value is not found");
        return *opt_index;
    }

    static constexpr auto value(size_t index) noexcept -> E
    {
        return magic_enum::enum_value<E>(index);
    }

    using Storage = std::bitset<capacity>;

public:
    class const_iterator final
    {
        friend class EnumFlags;
    public:

        using iterator_category = std::input_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = E;
        using reference         = E;

        const_iterator() noexcept = default;

        auto operator*() const noexcept
            -> reference
        {
            return value(index());
        }

        auto operator++() noexcept -> const_iterator&
        {
            auto index = std::countr_zero(storage_.to_ullong());
            assert(static_cast<unsigned int>(index) < capacity);
            storage_.set(index, 0);
            return *this;
        }

        auto operator++(int) noexcept -> const_iterator
        { auto tmp = *this; ++(*this); return tmp; }

        friend auto operator==(const const_iterator& a,
                               const const_iterator& b) noexcept
            -> bool = default;

    private:
        explicit const_iterator(Storage storage) noexcept :
            storage_{storage}
        {}

        const_iterator(Storage storage, size_t index) noexcept
        {
            auto bits = storage.to_ullong();
            bits &= ~((1ull << index) - 1ull);
            storage_ = Storage{bits};
        }

        auto index() const noexcept -> size_t
        {
            size_t result = std::countr_zero(storage_.to_ullong());
            assert(result < capacity);
            return result;
        }

        Storage storage_;
    };

    friend class const_iterator;
    using iterator = const_iterator;

    auto begin() const noexcept -> const_iterator
    {
        return const_iterator{ storage_ };
    }

    auto end() const noexcept -> const_iterator
    {
        return const_iterator{};
    }

    auto find(E flag) const -> const_iterator
    {
        auto i = index(flag);
        if (storage_.test(i))
            return const_iterator{storage_, i};

        return end();
    }

    auto insert(E flag) -> std::pair<iterator, bool>
    {
        auto i = index(flag);
        auto need_insert = !storage_.test(i);
        if (need_insert)
            storage_.set(i);
        return {iterator{storage_, i}, need_insert};
    }

    auto erase(iterator it)
    {
        storage_.set(it.index(), false);
        ++it;
        return it;
    }

private:

    Storage storage_;
};

template <typename E, std::same_as<E>... Ts>
EnumFlags(E, Ts... flags) -> EnumFlags<E>;



template <typename T>
constexpr inline bool is_enum_flags_type_v = false;

template <EnumType E>
constexpr inline bool is_enum_flags_type_v<EnumFlags<E>> = true;

template <typename T>
concept EnumFlagsType =
    is_enum_flags_type_v<T>;

template <typename T>
concept EnumFlagsOpArgType =
    std::is_enum_v<T> || is_enum_flags_type_v<T>;

template <typename T>
struct EnumOf_;

template <EnumType E>
struct EnumOf_<E>
{
    using type = E;
};

template <EnumType E>
struct EnumOf_<EnumFlags<E>>
{
    using type = E;
};

template <EnumFlagsOpArgType T>
using EnumOf = typename EnumOf_<T>::type;

template <typename T0, typename T1>
concept EnumFlagsOpArgs = requires
{
    requires EnumFlagsOpArgType<T0>;
    requires EnumFlagsOpArgType<T1>;
    requires std::same_as<EnumOf<T0>, EnumOf<T1>>;

    // Do not allow binary ops on bare enum arguments
    requires EnumFlagsType<T0> || EnumFlagsType<T1>;
};


template <EnumFlagsOpArgType T0, EnumFlagsOpArgType T1>
requires std::same_as<EnumOf<T0>, EnumOf<T1>>
auto operator&(const T0& lhs, const T1& rhs) noexcept
    -> EnumFlags<EnumOf<T0>>
{
    EnumFlags<EnumOf<T0>> result{lhs};
    result &= rhs;
    return result;
}

template <EnumFlagsOpArgType T0, EnumFlagsOpArgType T1>
    requires std::same_as<EnumOf<T0>, EnumOf<T1>>
auto operator|(const T0& lhs, const T1& rhs) noexcept
    -> EnumFlags<EnumOf<T0>>
{
    EnumFlags<EnumOf<T0>> result{lhs};
    result |= rhs;
    return result;
}

template <EnumFlagsOpArgType T0, EnumFlagsOpArgType T1>
    requires std::same_as<EnumOf<T0>, EnumOf<T1>>
auto operator^(const T0& lhs, const T1& rhs) noexcept
    -> EnumFlags<EnumOf<T0>>
{
    EnumFlags<EnumOf<T0>> result{lhs};
    result ^= rhs;
    return result;
}

} // namespace mpk::mix

template <mpk::mix::EnumType E>
struct MPKMIX_FORMAT_NS::formatter<mpk::mix::EnumFlags<E>> final
    : mpk::mix::OstreamFormatter<mpk::mix::EnumFlags<E>>
{};
