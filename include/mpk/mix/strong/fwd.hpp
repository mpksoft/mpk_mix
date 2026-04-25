/** @file
 * @brief Forward declarations and concepts for the Strong typedef framework
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <concepts>

namespace mpk::mix
{

// Forward declaration of `Strong`

template <typename Traits_>
struct Strong;

// The `StrongType` concept

template <typename T>
constexpr inline auto is_strong_v = false;

template <typename Traits>
constexpr inline auto is_strong_v<Strong<Traits>> = true;

template <typename T>
concept StrongType = is_strong_v<T>;

// Arithmetic / index / count

// Satisfied when arithmetic index/count semantic is implied
template <typename T>
concept StrongArithmeticType = StrongType<T> && T::arithmetic;

// Satisfied when the underlying type is an arithmetic one
template <typename T>
concept StrongNumericType = requires {
    requires StrongType<T>;
    requires std::is_arithmetic_v<typename T::Weak>;
};

template <typename T>
concept StrongCountType = StrongType<T> && T::is_count;

template <typename T>
concept StrongIndexType =
    StrongType<T> && T::is_index && StrongCountType<typename T::StrongDiff>;

template <typename T>
concept StrongStringType = StrongType<T> && T::is_string;

// Strong view (optional)

template <typename Features>
concept HasViewType = requires { typename Features::View; };

template <StrongType T>
struct StrongView_;

template <StrongType T>
using StrongView = StrongView_<T>::type;

template <StrongType T>
    requires HasViewType<typename T::Traits::Features>
struct StrongView_<T>
{
    using type = T::Traits::Features::View;
};

template <StrongType T>
    requires(!HasViewType<typename T::Traits::Features>)
struct StrongView_<T>
{
    using type = T;
};

} // namespace mpk::mix
