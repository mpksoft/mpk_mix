/** @file
 * @brief Fixed-capacity ring buffer
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>
#include <memory>
#include <span>
#include <stdexcept>


namespace mpk::mix {

template <typename T>
class RingBuffer {
public:
    template <typename ValueType>
    class Iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = ValueType;
        using difference_type = std::ptrdiff_t;
        using index_type = size_t;
        using pointer = ValueType*;
        using reference = ValueType&;

        Iterator() = default;

        Iterator(ValueType* data, size_t capacity, size_t index, size_t count)
            : data_(data),
              capacity_(capacity),
              index_(index),
              count_(count) {}

        reference operator*() const { return data_[index_]; }
        pointer operator->() const { return &data_[index_]; }

        Iterator& operator++() {
            assert(count_ > 0);
            assert(capacity_ > 0);
            ++index_;
            if (index_ == capacity_)
                index_ = 0;
            --count_;
            return *this;
        }

        Iterator& operator--() {
            assert(count_ < capacity_);
            assert(capacity_ > 0);
            if (index_ == 0)
                index_ = capacity_ - 1;
            else
                --index_;
            ++count_;
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        Iterator operator--(int) {
            Iterator tmp = *this;
            --(*this);
            return tmp;
        }

        auto operator+=(difference_type d) noexcept -> Iterator&
        {
            assert(static_cast<difference_type>(count_) >= d);
            index_ = (index_ + d) % capacity_;
            count_ -= d;
            return *this;
        }

        auto operator-=(difference_type d) noexcept -> Iterator&
        {
            assert(count_ + d <= capacity_);
            index_ = (index_ + capacity_ - d) % capacity_;
            count_ += d;
            return *this;
        }

        friend auto operator+(Iterator it, difference_type d) noexcept
            -> Iterator
        {
            it += d;
            return it;
        }

        friend auto operator-(Iterator it, difference_type d) noexcept
            -> Iterator
        {
            it -= d;
            return it;
        }

        friend auto operator-(Iterator lhs, Iterator rhs) noexcept
            -> difference_type
        {
            return rhs.count_ - lhs.count_;
        }

        auto operator[](difference_type d) -> reference
        {
            return data_[(index_ + d) % capacity_];
        }

        friend auto operator<=>(
            const Iterator& a, const Iterator& b) noexcept
            -> std::strong_ordering
        {
            assert(a.data_ == b.data_);
            return a.count_ <=> b.count_;
        }

        friend bool operator==(const Iterator& a, const Iterator& b)
        {
            assert(a.data_ == b.data_);
            return a.count_ == b.count_;
        }

    private:
        ValueType* data_{};
        size_t capacity_{};
        size_t index_{};
        size_t count_{};
    };

    using iterator = Iterator<T>;
    using const_iterator = Iterator<const T>;

    explicit RingBuffer(size_t capacity)
        : capacity_(capacity),
        data_(std::make_unique<T[]>(capacity)),
        head_(0),
        size_(0) {
        if (capacity == 0) throw std::invalid_argument("Capacity must be > 0");
    }

    T& front() { assert(size_ > 0); return data_[head_]; }
    const T& front() const {
        assert(size_ > 0);
        return data_[head_];
    }

    T& back() {
        assert(size_ > 0);
        return data_[(head_ + size_ - 1) % capacity_];
    }

    const T& back() const {
        assert(size_ > 0);
        return data_[(head_ + size_ - 1) % capacity_];
    }

    auto operator[](size_t index) -> T& {
        assert(index < size_);
        return data_[(head_ + index) % capacity_];
    }

    auto operator[](size_t index) const -> const T& {
        assert(index < size_);
        return data_[(head_ + index) % capacity_];
    }

    void push_back(const T& value) { push_impl(value); }
    void push_back(T&& value) { push_impl(std::move(value)); }

    void pop_front() {
        if (size_ > 0) {
            data_[head_] = T{};
            head_ = (head_ + 1) % capacity_;
            size_--;
        }
    }

    void clear() {
        while (!empty()) {
            pop_front();
        }
        head_ = 0;
        size_ = 0;
    }

    std::span<T> first_segment() {
        if (size_ == 0) return {};
        size_t count = std::min(size_, capacity_ - head_);
        return {&data_[head_], count};
    }

    std::span<const T> first_segment() const {
        if (size_ == 0) return {};
        size_t count = std::min(size_, capacity_ - head_);
        return {&data_[head_], count};
    }

    std::span<T> second_segment() {
        if (size_ == 0) return {};
        size_t first_count = std::min(size_, capacity_ - head_);
        size_t second_count = size_ - first_count;
        return second_count == 0
                   ? std::span<T>{}
                   : std::span<T>{&data_[0], second_count};
    }

    std::span<const T> second_segment() const {
        if (size_ == 0) return {};
        size_t first_count = std::min(size_, capacity_ - head_);
        size_t second_count = size_ - first_count;
        return second_count == 0
                   ? std::span<const T>{}
                   : std::span<const T>{&data_[0], second_count};
    }

    iterator begin() {
        return iterator(data_.get(), capacity_, head_, size_);
    }

    iterator end() {
        return iterator(data_.get(), capacity_, (head_ + size_)%capacity_, 0);
    }

    const_iterator begin() const {
        return const_iterator(data_.get(), capacity_, head_, size_);
    }

    const_iterator end() const {
        return const_iterator(
            data_.get(), capacity_, (head_ + size_)%capacity_, 0);
    }

    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }

private:
    template <typename U>
    void push_impl(U&& value) {
        size_t index = (head_ + size_) % capacity_;
        data_[index] = std::forward<U>(value);
        if (size_ < capacity_) {
            size_++;
        } else {
            head_ = (head_ + 1) % capacity_;
        }
    }

    size_t capacity_;
    std::unique_ptr<T[]> data_;
    size_t head_;
    size_t size_;
};

} // namespace mpk::mix
