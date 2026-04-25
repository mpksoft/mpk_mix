/** @file
 * @brief Subscribers<Callback> maintains and invokes a list of callbacks
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <ranges>
#include <stdexcept>
#include <vector>

namespace mpk::mix
{

template <typename Callback>
class Subscribers final
{
public:
    auto add(Callback cb) -> void
    {
        callbacks_.push_back(cb);
    }

    auto remove(Callback cb) -> void
        requires requires(Callback a, Callback b) {
            { a == b } -> std::same_as<bool>;
        }
    {
        auto it = std::ranges::find(callbacks_, cb);
        if (it == callbacks_.end())
            throw std::runtime_error("Callback being removed is not found");
        callbacks_.erase(it);
    }

    auto empty() const noexcept -> bool
    {
        return callbacks_.empty();
    }

    template <typename... Args>
    auto operator()(Args&&... args) const -> void
    {
        for (auto const& cb: callbacks_)
            cb(std::forward<Args>(args)...);
    }

private:
    std::vector<Callback> callbacks_;
};

} // namespace mpk::mix
