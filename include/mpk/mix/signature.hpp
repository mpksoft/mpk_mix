/** @file
 * @brief SignatureOf, MethodOf, and related function-signature type traits
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/signature_fwd.hpp"
#include "mpk/mix/meta/type.hpp"

#include <type_traits>

namespace mpk::mix
{

template <typename M, typename Cls>
struct SignatureOf;

template <typename M, typename Cls>
using SignatureOf_t = typename SignatureOf<M, Cls>::type;

template <typename R, typename... A, bool NX>
struct SignatureOf<R (*)(A...) noexcept(NX), void>
{
    using type = R(A...) noexcept(NX);
};

template <typename R, typename... A, bool NX, typename Cls>
struct SignatureOf<R (Cls::*)(A...) noexcept(NX), Cls>
{
    using type = R(A...) noexcept(NX);
};

template <typename R, typename... A, bool NX, typename Cls>
struct SignatureOf<R (Cls::*)(A...) & noexcept(NX), Cls>
{
    using type = R(A...) noexcept(NX);
};

template <typename R, typename... A, bool NX, typename Cls>
struct SignatureOf<R (std::remove_const_t<Cls>::*)(A...) const noexcept(NX), Cls>
{
    using type = R(A...) noexcept(NX);
};

template <typename R, typename... A, bool NX, typename Cls>
struct SignatureOf<R (std::remove_const_t<Cls>::*)(A...) const & noexcept(NX), Cls>
{
    using type = R(A...) noexcept(NX);
};

// ---

template <typename M, typename Cls>
constexpr inline auto is_method_of = false;

template <typename R, typename... A, bool NX, typename Cls>
constexpr inline auto is_method_of<R (Cls::*)(A...) noexcept(NX), Cls> = true;

template <typename R, typename... A, bool NX, typename Cls>
constexpr inline auto is_method_of<R (Cls::*)(A...) & noexcept(NX), Cls> = true;

template <typename R, typename... A, bool NX, typename Cls>
constexpr inline auto
    is_method_of<R (std::remove_const_t<Cls>::*)(A...) const noexcept(NX), Cls> =
        true;

template <typename R, typename... A, bool NX, typename Cls>
constexpr inline auto
    is_method_of<R (std::remove_const_t<Cls>::*)(A...) const & noexcept(NX), Cls> =
        true;

template <typename M, typename Cls>
concept MethodOf = is_method_of<M, Cls>;

// ---

template <typename M, typename Cls, typename S>
constexpr inline auto is_method_with_signature_of = false;

// Non-const non-ref method: check inheritance (Cls may derive from MCls)
template <typename R, typename... A, bool NX, typename MCls, typename Cls>
constexpr inline auto is_method_with_signature_of<
    R (MCls::*)(A...) noexcept(NX),
    Cls,
    R(A...) noexcept(NX)> = std::is_base_of_v<MCls, Cls>;

template <typename R, typename... A, bool NX, typename MCls, typename Cls>
constexpr inline auto is_method_with_signature_of<
    R (MCls::*)(A...) & noexcept(NX),
    Cls,
    R(A...) noexcept(NX)> = std::is_base_of_v<MCls, Cls>;

template <typename R, typename... A, bool NX, typename Cls>
constexpr inline auto is_method_with_signature_of<
    R (std::remove_const_t<Cls>::*)(A...) const noexcept(NX),
    Cls,
    R(A...) noexcept(NX)> = true;

template <typename R, typename... A, bool NX, typename Cls>
constexpr inline auto is_method_with_signature_of<
    R (std::remove_const_t<Cls>::*)(A...) const & noexcept(NX),
    Cls,
    R(A...) noexcept(NX)> = true;

template <typename M, typename Cls, typename S>
concept MethodWithSignatureOf = is_method_with_signature_of<M, Cls, S>;

// ---

template <SignatureType S>
constexpr auto signature(S*) noexcept -> Type_Tag<S>
{
    return {};
}

template <typename R, typename... A, bool NX, typename Cls>
constexpr auto signature(R (Cls::*)(A...) noexcept(NX)) noexcept
    -> Type_Tag<R(A...) noexcept(NX)>
{
    return {};
}

template <typename R, typename... A, bool NX, typename Cls>
constexpr auto signature(R (Cls::*)(A...) & noexcept(NX)) noexcept
    -> Type_Tag<R(A...) noexcept(NX)>
{
    return {};
}

template <typename R, typename... A, bool NX, typename Cls>
constexpr auto signature(R (Cls::*)(A...) const noexcept(NX)) noexcept
    -> Type_Tag<R(A...) noexcept(NX)>
{
    return {};
}

template <typename R, typename... A, bool NX, typename Cls>
constexpr auto signature(R (Cls::*)(A...) const& noexcept(NX)) noexcept
    -> Type_Tag<R(A...) noexcept(NX)>
{
    return {};
}

// ---

template <typename R, typename... A, bool NX>
constexpr auto return_type(Type_Tag<R(A...) noexcept(NX)>) -> Type_Tag<R>
{
    return {};
}

template <NonTypeTagType T>
constexpr auto return_type(T v)
{
    return return_type(signature(v));
}

template <typename>
struct ReturnType;

template <typename T>
using ReturnType_t = typename ReturnType<T>::type;

template <typename R, typename... A, bool NX>
struct ReturnType<R(A...) noexcept(NX)>
{
    using type = R;
};

// ---

template <typename R, typename... A, bool NX>
constexpr auto arity(Type_Tag<R(A...) noexcept(NX)>) -> unsigned
{
    return sizeof...(A);
}

template <NonTypeTagType T>
constexpr auto arity(T v) -> unsigned
{
    return arity(signature(v));
}

template <typename R, typename A0, typename... A, bool NX>
constexpr auto arg_0_type(Type_Tag<R(A0, A...) noexcept(NX)>) -> Type_Tag<A0>
{
    return {};
}

template <NonTypeTagType T>
constexpr auto arg_0_type(T v)
{
    return arg_0_type(signature(v));
}

} // namespace mpk::mix
