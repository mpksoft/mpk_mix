/** @file
 * @brief Mutable/Const mutability tags and maybe_const<T,B> helper
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/util/any_of.hpp"

#include <type_traits>

namespace mpk::mix
{

struct Constant
{
    template <typename T>
    using alias = std::add_const_t<T>;
};

struct Mutable
{
    template <typename T>
    using alias = std::remove_const_t<T>;
};

template <template <typename> typename M, typename T>
concept MutabilityAliasFor = requires {
    requires AnyOf<M<T>, std::add_const_t<T>, std::remove_const_t<T>>;
    requires std::same_as<M<M<T>>, M<T>>;
};

template <template <typename> typename M, typename... Ts>
concept MutabilityAliasForAllOf = (MutabilityAliasFor<M, Ts> && ...);

template <template <typename T> typename M>
concept MutabilityAlias = MutabilityAliasForAllOf<M, int, char, double>;

static_assert(MutabilityAlias<Constant::alias>);
static_assert(MutabilityAlias<Mutable::alias>);

template <typename T>
concept MutabilityType = requires {
    typename T::template alias<int>;
    requires MutabilityAlias<T::template alias>;
};

static_assert(MutabilityType<Constant>);
static_assert(MutabilityType<Mutable>);

// clang-format off
template <typename T>
using MutabilityOf =
    std::conditional_t<
        std::is_const_v<std::remove_reference_t<T>>, Constant, Mutable>;
// clang-format on

template <MutabilityType M, typename T>
using MutabilityAs = typename M::template alias<T>;

template <typename Src, typename T>
    requires(!MutabilityType<Src>)
using MutabilityLike = MutabilityAs<MutabilityOf<Src>, T>;

} // namespace mpk::mix
