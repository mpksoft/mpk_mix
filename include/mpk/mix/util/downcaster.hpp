/** @file
 * @brief RTTI-based downcast with assertion on type mismatch
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

namespace mpk::mix
{

template <typename Derived, typename Base>
struct Downcaster
{
    auto operator()(Base* base) const noexcept -> Derived*
    {
        return static_cast<Derived*>(base);
    }

    auto operator()(const Base* base) const noexcept -> const Derived*
    {
        return static_cast<const Derived*>(base);
    }
};

} // namespace mpk::mix
