/** @file
 * @brief Strong<Traits> implementation with composable feature mix-ins
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/macro.hpp"
#include "mpk/mix/meta/nil.hpp"
#include "mpk/mix/meta/type.hpp"
#include "mpk/mix/meta/zero.hpp"
#include "mpk/mix/strong/fwd.hpp"

#include <cassert>
#include <format>
#include <iostream>
#include <limits>
#include <type_traits>
#include <utility>

namespace mpk::mix
{

template <typename Traits_>
struct Strong final
{
    using Traits = Traits_;
    using Self = Strong<Traits>;
    using Weak = typename Traits::Weak;
    using View = StrongView<Self>;

    using StrongDiff = typename Traits::Features::StrongDiff;

    using Ordering = std::conditional_t<
        std::is_floating_point_v<Weak>,
        std::partial_ordering,
        std::strong_ordering>;

    static constexpr auto arithmetic = Traits::Features::arithmetic;

    static constexpr auto is_count =
        arithmetic && std::same_as<StrongDiff, Nil_Tag>;

    static constexpr auto is_index =
        arithmetic && !std::same_as<StrongDiff, Nil_Tag>;

    static constexpr auto is_string = requires {
        Traits::Features::string;
        Traits::Features::string == true;
    };

    template <StrongType Index>
    static constexpr bool is_adjacent_index =
        is_count && Index::is_index
        && std::same_as<typename Index::StrongDiff, Self>;

    Weak v = Traits::default_value();

    constexpr Strong() noexcept = default;

    constexpr explicit Strong(Weak v) : v{std::move(v)}
    {
    }

    template <typename View>
        requires requires {
            typename Traits::Features::View;
            requires std::same_as<
                std::remove_cvref_t<View>,
                typename Traits::Features::View>;
        }
    constexpr Strong(View view) : v{view.v}
    {
    }

    constexpr /* implicit */ Strong(Zero_Tag)
        requires std::default_initializable<Weak>
      : v{}
    {
    }

    Strong(Strong const&) = default;
    Strong(Strong&&) = default;
    auto operator=(Strong const&) -> Strong& = default;
    auto operator=(Strong&&) -> Strong& = default;

    auto operator=(Zero_Tag) -> Strong&
        requires std::assignable_from<Weak&, Weak const&>
    {
        v = Weak{};
        return *this;
    }

    auto view() const -> View
    {
        return View{typename View::Weak{v}};
    }

    static constexpr auto max() noexcept -> Self
        requires arithmetic
    {
        return Self{std::numeric_limits<Weak>::max()};
    }

    constexpr auto operator-() const noexcept -> Self
        requires is_count && std::is_signed_v<Weak>
    {
        return Self{-v};
    }

    constexpr auto operator+(Self that) const noexcept -> Self
        requires is_count
    {
        return Self{static_cast<Weak>(v + that.v)};
    }

    template <StrongType Index>
        requires is_adjacent_index<Index>
    constexpr auto operator+(Index that) const noexcept -> Index
    {
        return Index{static_cast<Weak>(v + that.v)};
    }

    constexpr auto operator-(Self that) const noexcept -> Self
        requires is_count
    {
        return Self{static_cast<Weak>(v - that.v)};
    }

    auto operator+=(Self that) noexcept -> Self&
        requires is_count
    {
        v += that.v;
        return *this;
    }

    auto operator-=(Self that) noexcept -> Self&
        requires is_count
    {
        v -= that.v;
        return *this;
    }

    constexpr auto operator+(StrongDiff that) const noexcept -> Self
        requires is_index
    {
        return Self{static_cast<Weak>(v + that.v)};
    }

    constexpr auto operator-(StrongDiff that) const noexcept -> Self
        requires is_index
    {
        return Self{static_cast<Weak>(v - that.v)};
    }

    constexpr auto operator-(Self that) const noexcept -> StrongDiff
        requires is_index
    {
        return StrongDiff{static_cast<Weak>(v - that.v)};
    }

    auto operator+=(StrongDiff that) noexcept -> Self&
        requires is_index
    {
        v += that.v;
        return *this;
    }

    auto operator-=(StrongDiff that) noexcept -> Self&
        requires is_index
    {
        v -= that.v;
        return *this;
    }

    template <std::convertible_to<Weak> Factor>
        requires is_count
    auto operator*=(Factor rhs) noexcept -> Self&
    {
        v *= rhs;
        return *this;
    }

    template <std::convertible_to<Weak> Factor>
        requires is_count
    auto operator/=(Factor rhs) noexcept -> Self&
    {
        v /= rhs;
        return *this;
    }

    auto operator++() noexcept -> Self
        requires arithmetic
    {
        ++v;
        return *this;
    }

    auto operator++(int) noexcept -> Self
        requires arithmetic
    {
        auto result = *this;
        ++v;
        return result;
    }

    auto operator--() noexcept -> Self
        requires arithmetic
    {
        --v;
        return *this;
    }

    auto operator--(int) noexcept -> Self
        requires arithmetic
    {
        auto result = *this;
        --v;
        return result;
    }

    constexpr explicit operator Weak&() noexcept
    {
        return v;
    }

    constexpr explicit operator Weak const&() const noexcept
    {
        return v;
    }

    operator View() const noexcept
        requires(!std::same_as<View, Self>)
    {
        return view();
    }

    auto operator<=>(Self const&) const noexcept -> Ordering = default;

    auto operator<=>(View const& that) const noexcept -> Ordering
        requires HasViewType<typename Traits::Features>
    {
        return view() <=> that;
    }

    auto operator==(Self const& that) const noexcept -> bool = default;

    auto operator==(View const& that) const noexcept -> bool
        requires HasViewType<typename Traits::Features>
    {
        return view() == that;
    }
};

template <StrongCountType T, std::convertible_to<typename T::Weak> Factor>
constexpr auto operator*(T lhs, Factor rhs) noexcept -> T
{
    return T{lhs.v * rhs};
}

template <StrongCountType T, std::convertible_to<typename T::Weak> Factor>
constexpr auto operator*(Factor lhs, T rhs) noexcept -> T
{
    return T{lhs * rhs.v};
}

template <StrongCountType T, std::convertible_to<typename T::Weak> Factor>
constexpr auto operator/(T lhs, Factor rhs) noexcept -> T
{
    return T{lhs.v / rhs};
}

// ---

template <typename Weak_, typename Features_>
struct StrongTraits
{
    using Weak = Weak_;
    static constexpr auto default_value() noexcept -> Weak
    {
        return {};
    }
    using Features = Features_;
};

template <typename Weak_, typename Features_, Weak_ default_value_>
struct StrongWithDefaultTraits
{
    using Weak = Weak_;
    static constexpr auto default_value() noexcept -> Weak
    {
        return default_value_;
    }
    using Features = Features_;
};

// ---

struct StrongIdFeatures final
{
    using StrongDiff = Nil_Tag;

    static constexpr bool arithmetic = false;
};

struct StrongCountFeatures final
{
    using StrongDiff = Nil_Tag;

    static constexpr bool arithmetic = true;
};

template <StrongType Count>
struct StrongIndexFeatures final
{
    using StrongDiff = Count;

    static constexpr bool arithmetic = true;
};

template <StrongType View_>
struct StrongStringFeatures final
{
    using StrongDiff = Nil_Tag;
    using View = View_;

    static constexpr bool arithmetic = false;
    static constexpr bool string = true;
};

// ---

template <StrongType T>
auto operator<<(std::ostream& s, T const& x) -> std::ostream&
{
    return s << x.v;
}

// ---

template <typename T>
    requires(!StrongType<T> && !std::is_const_v<T>)
constexpr auto raw(T& x) noexcept -> T&
{
    return x;
}

template <typename T>
    requires(!StrongType<T>)
constexpr auto raw(T const& x) noexcept -> T const&
{
    return x;
}

template <StrongType T>
constexpr auto raw(T& x) noexcept -> typename T::Weak&
{
    return x.v;
}

template <StrongType T>
constexpr auto raw(T const& x) noexcept -> typename T::Weak const&
{
    return x.v;
}

} // namespace mpk::mix

template <mpk::mix::StrongType T>
struct std::formatter<T> : std::formatter<typename T::Weak>
{
    auto format(const T& x, std::format_context& ctx) const
    {
        return std::formatter<typename T::Weak>::format(x.v, ctx);
    }
};

#define MPKMIX_STRONG_TYPE(Name, Weak_, ...)                        \
    struct Name##_StrongTraits final                                \
      : ::mpk::mix::StrongTraits<                                   \
            Weak_,                                                  \
            MPKMIX_DEFAULT_A0_TO(                                   \
                ::mpk::mix::StrongIdFeatures, ##__VA_ARGS__)>       \
    {                                                               \
    };                                                              \
    using Name = ::mpk::mix::Strong<Name##_StrongTraits>

#define MPKMIX_STRONG_TYPE_WITH_DEFAULT(Name, Weak_, Default, ...)  \
    struct Name##_StrongTraits final                                \
      : ::mpk::mix::StrongWithDefaultTraits<                        \
            Weak_,                                                  \
            MPKMIX_DEFAULT_A0_TO(                                   \
                ::mpk::mix::StrongIdFeatures, ##__VA_ARGS__),       \
            Default>                                                \
    {                                                               \
    };                                                              \
    using Name = ::mpk::mix::Strong<Name##_StrongTraits>

#define MPKMIX_STRONG_LITERAL_SUFFIX(Name, suffix)                  \
    constexpr inline Name operator"" suffix(unsigned long long value) \
    {                                                               \
        assert(value <= std::numeric_limits<Name::Weak>::max());    \
        return Name{static_cast<Name::Weak>(value)};                \
    }                                                               \
    static_assert(true)

#define MPKMIX_STRONG_STRING_VIEW(Name) \
    MPKMIX_STRONG_TYPE(Name, std::string_view)

#define MPKMIX_STRONG_STRING(Name)            \
    MPKMIX_STRONG_STRING_VIEW(Name##View);    \
    MPKMIX_STRONG_TYPE(                       \
        Name,                                 \
        std::string,                          \
        ::mpk::mix::StrongStringFeatures<Name##View>)
