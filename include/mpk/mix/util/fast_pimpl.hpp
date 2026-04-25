/** @file
 * @brief FastPimpl<T,Size,Align> in-place pimpl that avoids heap allocation
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <new>
#include <utility>


namespace mpk::mix {

template <typename T, std::size_t Size, std::size_t Alignment = alignof(void*)>
class FastPimpl {
public:
    FastPimpl() noexcept : initialized_(false) {}

    ~FastPimpl() {
        destroy();
    }

    FastPimpl(const FastPimpl&) = delete;
    FastPimpl& operator=(const FastPimpl&) = delete;
    FastPimpl(FastPimpl&&) = delete;
    FastPimpl& operator=(FastPimpl&&) = delete;

    template <typename... Args>
    void emplace(Args&&... args) {
        static_assert(sizeof(T) <= Size,
                      "FastPimpl Error: Storage Size is too small for the Implementation type.");
        static_assert(alignof(T) <= Alignment,
                      "FastPimpl Error: Alignment requirement of T is stricter than storage.");

        assert(!initialized_ && "FastPimpl already initialized!");

        new (&storage_) T(std::forward<Args>(args)...);
        initialized_ = true;
    }

    void destroy() {
        if (initialized_) {
            ptr()->~T();
            initialized_ = false;
        }
    }

    T* operator->() { return ptr(); }
    const T* operator->() const { return ptr(); }

    T& operator*() { return *ptr(); }
    const T& operator*() const { return *ptr(); }

    bool has_value() const { return initialized_; }

private:
    T* ptr() {
        return std::launder(reinterpret_cast<T*>(&storage_));
    }

    const T* ptr() const {
        return std::launder(reinterpret_cast<const T*>(&storage_));
    }

    alignas(Alignment) std::byte storage_[Size];
    bool initialized_;
};

} // namespace mpk::mix
