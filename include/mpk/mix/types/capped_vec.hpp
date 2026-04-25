/** @file
 * @brief CappedVec<T,N> fixed-capacity heap-free vector
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>
#include <utility>

namespace mpk::mix
{

template <typename T, size_t N>
class CappedVec
{
public:
    using value_type = T;
    using Storage = std::array<T, N>;
    using iterator = typename Storage::iterator;
    using const_iterator = typename Storage::const_iterator;

    CappedVec() = default;

    auto size() const { return size_; }
    constexpr auto capacity() const { return N; }

    iterator begin() { return data_.begin(); }
    iterator end() { return data_.begin() + size_; }
    const_iterator cbegin() const { return data_.begin(); }
    const_iterator cend() const { return data_.begin() + size_; }
    const_iterator begin() const { return data_.begin(); }
    const_iterator end() const { return data_.begin() + size_; }

    auto operator[](size_t i) -> T& { return data_[i]; }
    auto operator[](size_t i) const -> T const& { return data_[i]; }

    auto at(size_t i) -> T& { check_bounds(i); return data_[i]; }
    auto at(size_t i) const -> T const& { check_bounds(i); return data_[i]; }

    auto front() -> T& { check_not_empty(); return data_[0]; }
    auto front() const -> T const& { check_not_empty(); return data_[0]; }

    auto back() -> T& { check_not_empty(); return data_[size_ - 1]; }
    auto back() const -> T const& { check_not_empty(); return data_[size_ - 1]; }

    auto push_back(T const& value) -> void
    {
        check_capacity();
        data_[size_++] = value;
    }

    auto push_back(T&& value) -> void
    {
        check_capacity();
        data_[size_++] = std::move(value);
    }

    auto pop_back() -> void
    {
        check_not_empty();
        back() = {};
        --size_;
    }

    auto resize(size_t size, T fill = {}) -> void
    {
        if (size > N)
            throw std::range_error("CappedVec: requested size exceeds capacity");
        auto old_size = std::exchange(size_, size);
        for (auto i = old_size; i < size_; ++i)
            data_[i] = fill;
        for (auto i = size_; i < old_size; ++i)
            data_[i] = {};
    }

    auto clear() -> void
    {
        resize(0);
    }

private:
    auto check_bounds(size_t i) const -> void
    {
        if (i >= size_)
            throw std::range_error("CappedVec: index is out of range");
    }

    auto check_capacity() const -> void
    {
        if (size_ == N)
            throw std::range_error(
                "CappedVec: cannot add more elements because capacity is exhausted");
    }

    auto check_not_empty() const -> void
    {
        if (size_ == 0)
            throw std::range_error("CappedVec: vector is empty");
    }

    Storage data_;
    size_t size_{};
};

} // namespace mpk::mix
