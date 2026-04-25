/** @file
 * @brief RAII guard that increments an integer on entry and decrements on exit
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

namespace mpk::mix
{

template <typename T>
class ScopedInc
{
public:
    explicit ScopedInc(T& value) : value_{value}
    {
        ++value_;
    }

    ~ScopedInc()
    {
        --value_;
    }

    ScopedInc(ScopedInc const&) = delete;
    auto operator=(ScopedInc const&) -> ScopedInc& = delete;

private:
    T& value_;
};

} // namespace mpk::mix
