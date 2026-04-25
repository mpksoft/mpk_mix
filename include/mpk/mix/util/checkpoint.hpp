/** @file
 * @brief Checkpoint<T> tracks value history for undo/redo or auditing
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/util/linked_list.hpp"

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>


namespace mpk::mix {

template <typename T>
concept UpdateHistoryType = requires(T update_history)
{
    requires std::is_default_constructible_v<T>;
    requires std::is_copy_constructible_v<T>;
    update_history.reset();
    update_history.clear();
};

template <UpdateHistoryType UpdateHistory>
class Checkpoint final :
    public mpk::mix::IntrusiveLinkedListConnectivity<Checkpoint<UpdateHistory>>
{
public:
    template<std::invocable<UpdateHistory&> F>
    auto update(F&& f) -> void
    {
        std::invoke(std::forward<F>(f), update_history_);
    }

    auto sync() -> UpdateHistory
    {
        auto result = update_history_;
        update_history_.clear();
        return result;
    }

private:
    UpdateHistory update_history_;
};

} // namespace mpk::mix
