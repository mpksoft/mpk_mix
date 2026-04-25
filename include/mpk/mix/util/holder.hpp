/** @file
 * @brief ValueHolder<T> wrapper that stores a value and exposes it uniformly
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <concepts>
#include <type_traits>
#include <utility>


namespace mpk::mix {

template <typename T>
class ValueHolder
{
public:
    using value_type = T;
    static_assert(!std::is_reference_v<T>);

    template <typename... Args>
    requires (!(std::is_base_of_v<ValueHolder<T>, std::decay_t<Args>> || ...))
    ValueHolder(Args&&... args) :
        value_{ std::forward<Args>(args)... }
    {}

    ValueHolder(const ValueHolder&) = default;
    auto operator=(const ValueHolder&) -> ValueHolder& = default;
    ValueHolder(ValueHolder&&) = default;
    auto operator=(ValueHolder&&) -> ValueHolder& = default;

    auto value() noexcept -> T&
    { return value_; }

    auto value() const noexcept -> const T&
    { return value_; }

private:
    T value_;
};

template <typename T>
class ReferenceHolder
{
public:
    using value_type = T;
    static_assert(!std::is_reference_v<T>);

    ReferenceHolder(T& value) :
        value_{ value }
    {}

    ReferenceHolder(const ReferenceHolder&) = delete;
    auto operator=(const ReferenceHolder&) -> ReferenceHolder& = delete;
    ReferenceHolder(ReferenceHolder&&) = delete;
    auto operator=(ReferenceHolder&&) -> ReferenceHolder& = delete;

    auto value() noexcept -> T& { return value_; }
    auto value() const noexcept -> const T& { return value_; }

private:
    T& value_;
};

template <typename T>
class BaseOf
{
public:
    using value_type = T;
    static_assert(!std::is_reference_v<T>);

    auto value() noexcept -> T&
    { return *static_cast<T*>(this); }

    auto value() const noexcept -> const T&
    { return *static_cast<const T*>(this); }
};

template <typename T>
concept HolderType = requires(T holder, const T const_holder)
{
    typename T::value_type;

    { holder.value() } -> std::same_as<typename T::value_type&>;
    { const_holder.value() } -> std::same_as<const typename T::value_type&>;
};

} // namespace mpk::mix
