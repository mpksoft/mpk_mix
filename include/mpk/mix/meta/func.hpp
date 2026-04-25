/** @file
 * @brief Method class and signature deduction helpers
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/meta/type.hpp"
#include "mpk/mix/signature.hpp"

#include <type_traits>

namespace mpk::mix
{

namespace detail
{

template <typename C, typename R, typename... Args, bool NX>
constexpr auto method_class_f(R (C::*)(Args...) noexcept(NX)) noexcept -> Type_Tag<C>
{
    return {};
}

template <typename C, typename R, typename... Args, bool NX>
constexpr auto method_class_f(R (C::*)(Args...) const noexcept(NX)) noexcept -> Type_Tag<C>
{
    return {};
}

} // namespace detail


template <auto* func_>
    requires requires { signature(func_); }
struct Func_Tag final
{
    static constexpr auto func = func_;
    // signature() returns Type_Tag<S>; unwrap to get the raw function type.
    using Signature = typename decltype(signature(func_))::type;
};

template <auto* func>
    requires requires { signature(func); }
constexpr inline auto Func = Func_Tag<func>{};


template <auto method_>
struct Method_Tag final
{
    static constexpr auto method = method_;
    using Signature = typename decltype(signature(method_))::type;
    using MethodClass = typename decltype(detail::method_class_f(method_))::type;
};

template <auto method>
    requires requires { signature(method); }
constexpr inline auto Method = Method_Tag<method>{};

} // namespace mpk::mix
