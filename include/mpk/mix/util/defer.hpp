/** @file
 * @brief defer{} RAII wrapper that calls a callable on scope exit
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <cassert>
#include <concepts>
#include <utility>

namespace mpk::mix
{

template <std::invocable F>
class [[nodiscard]] Defer
{
public:
    explicit Defer(F&& f) : f_{std::forward<F>(f)}
    {
    }

    Defer(Defer const&) = delete;
    auto operator=(Defer const&) -> Defer& = delete;

    ~Defer() noexcept(false)
    {
        if (!committed_)
            f_();
    }

    auto commit() -> void
    {
        assert(!committed_);
        committed_ = true;
        f_();
    }

private:
    F f_;
    bool committed_{false};
};

} // namespace mpk::mix
