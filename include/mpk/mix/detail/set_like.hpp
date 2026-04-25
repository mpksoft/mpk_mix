/** @file
 * @brief SetLikeType concept — a type that behaves like a set of its own elements.
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <concepts>
#include <utility>

namespace mpk::mix::detail {

template <typename T>
concept SetLikeType = requires(T x, const T c)
{
    typename T::key_type;
    typename T::value_type;
    typename T::iterator;
    typename T::const_iterator;
    requires std::same_as<typename T::key_type, typename T::value_type>;

    { c.contains(std::declval<typename T::value_type>()) }
          -> std::same_as<bool>;
    { c.empty() } -> std::same_as<bool>;

    { x.begin() } -> std::same_as<typename T::iterator>;
    { c.begin() } -> std::same_as<typename T::const_iterator>;
    { x.end() } -> std::same_as<typename T::iterator>;
    { c.end() } -> std::same_as<typename T::const_iterator>;

    { c.find(std::declval<typename T::value_type>()) }
          -> std::same_as<typename T::const_iterator>;
    { x.insert(std::declval<typename T::value_type>()) }
          -> std::same_as<std::pair<typename T::iterator, bool>>;
    { x.erase(std::declval<typename T::iterator>()) }
          -> std::same_as<typename T::iterator>;
};

} // namespace mpk::mix::detail
