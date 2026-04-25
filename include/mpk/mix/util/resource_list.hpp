/** @file
 * @brief Type-erased list of owned resources released on destruction
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <list>
#include <memory>
#include <type_traits>

namespace mpk::mix
{

class ResourceList
{
public:
    ResourceList() noexcept = default;

    ResourceList(ResourceList const&) = delete;
    auto operator=(ResourceList const&) -> ResourceList& = delete;

    ResourceList(ResourceList&&) noexcept = default;
    auto operator=(ResourceList&&) noexcept -> ResourceList& = default;

    ~ResourceList()
    {
        clear();
    }

    template <typename T, typename... Args>
        requires(!std::is_unbounded_array_v<T> && !std::is_bounded_array_v<T>)
    auto make(Args&&... args) -> T*
    {
        return consume(std::make_shared<T>(std::forward<Args>(args)...));
    }

    template <typename T>
        requires std::is_bounded_array_v<T>
    auto make() -> std::remove_extent_t<T>*
    {
        using Element = std::remove_extent_t<T>;
        constexpr auto size = sizeof(T) / sizeof(Element);
        return make_array<Element>(size);
    }

    template <typename T>
        requires std::is_unbounded_array_v<T>
    auto make(size_t size) -> std::remove_extent_t<T>*
    {
        using Element = std::remove_extent_t<T>;
        return make_array<Element>(size);
    }

    auto operator()(std::shared_ptr<void> res) -> void
    {
        data_.push_back(std::move(res));
    }

    auto clear() noexcept -> void
    {
        for (auto it = data_.rbegin(), end = data_.rend(); it != end; ++it)
            it->reset();
    }

private:
    std::list<std::shared_ptr<void>> data_;

    template <typename Element>
    auto make_array(size_t size) -> Element*
    {
        return consume(std::make_unique<Element[]>(size));
    }

    template <typename P>
    auto consume(P res) -> decltype(res.get())
    {
        auto result = res.get();
        data_.push_back(std::move(res));
        return result;
    }
};

} // namespace mpk::mix
