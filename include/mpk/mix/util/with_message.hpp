/** @file
 * @brief WithMessage — FuncRef wrapper that stores a diagnostic message.
 *
 * The wrapped function is invoked transparently; the stored message string
 * is available for use by higher-level code (e.g. logging shims).
 * This base class does not perform any logging itself.
 *
 * Copyright (C) 2024 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/func_ref/func_ref.hpp"
#include "mpk/mix/func_ref/tags.hpp"

#include <string>


namespace mpk::mix {

template <bool NX, typename R, typename... Args>
class WithMessage
{
public:
    using Self =
        WithMessage<NX, R, Args...>;

    using Func =
        FuncRef<R(Args...) noexcept(NX)>;

    WithMessage() = default;

    WithMessage(Func func, std::string message)
        : func_{ func }
        , message_{ std::move(message) }
    {}

    auto set(Func func, std::string message)
    { *this = Self{ func, std::move(message) }; }

    auto empty() const noexcept
        -> bool
    { return func_.empty(); }

    auto func() const noexcept
        -> Func
    { return { Const<&Self::func_impl>, this }; }

    operator Func() const noexcept
    { return func(); }

    auto message() const noexcept
        -> const std::string&
    { return message_; }

private:
    auto func_impl(Args... args) const noexcept(NX)
        -> R
    { return func_(std::forward<Args>(args)...); }

    Func func_;
    std::string message_;
};

} // namespace mpk::mix
