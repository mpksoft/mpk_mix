/** @file
 * @brief Intrusive doubly-linked list with stable node addresses
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/util/holder.hpp"

#include <iterator>


namespace mpk::mix {

template <typename T, HolderType Holder>
class LinkedListConnectivity : public Holder
{
public:
    using connectivity_type = LinkedListConnectivity;

    template <typename... Args>
    LinkedListConnectivity(Args&&... args) :
        Holder{ std::forward<Args>(args)... },
        prev_{ this },
        next_{ this }
    {}

    ~LinkedListConnectivity()
    { unlink(); }

    LinkedListConnectivity(const LinkedListConnectivity&) = delete;
    auto operator=(const LinkedListConnectivity&)
        -> LinkedListConnectivity& = delete;

    LinkedListConnectivity(LinkedListConnectivity&& that) :
        Holder{ std::move(that) },
        prev_{ that.prev_ },
        next_{ that.next_ }
    {
        next_->prev_ = this;
        prev_->next_ = this;
        that.prev_ = &that;
        that.next_ = &that;
    }

    auto operator=(LinkedListConnectivity&& that)
        -> LinkedListConnectivity&
    {
        if (&that != this) {
            unlink();
            prev_ = that.prev_;
            next_ = that.next_;
            next_->prev_ = this;
            prev_->next_ = this;
            that.prev_ = &that;
            that.next_ = &that;
        }
        return *this;
    }

    auto link_to(LinkedListConnectivity& that) noexcept -> void
    {
        unlink();
        prev_ = that.prev_;
        next_ = &that;
        prev_->next_ = this;
        next_->prev_ = this;
    }

    auto unlink() noexcept -> void
    {
        if (prev_ == this)
            return;

        prev_->next_ = next_;
        next_->prev_ = prev_;
        prev_ = this;
        next_ = this;
    }

    auto operator*() -> T&
    { return this->value(); }

    auto operator*() const -> const T&
    { return this->value(); }

    auto operator->() -> T*
    { return &this->value(); }

    auto operator->() const -> const T*
    { return &this->value(); }

    auto prev() -> LinkedListConnectivity&
    { return *prev_; }

    auto prev() const -> const LinkedListConnectivity&
    { return *prev_; }

    auto next() -> LinkedListConnectivity&
    { return *next_; }

    auto next() const -> const LinkedListConnectivity&
    { return *next_; }

    auto linked_list_connectivity() -> LinkedListConnectivity&
    { return *this; }

    auto linked_list_connectivity() const -> const LinkedListConnectivity&
    { return *this; }

private:
    LinkedListConnectivity* prev_;
    LinkedListConnectivity* next_;
};

namespace detail {

template <typename T>
constexpr inline auto LinkedListConnectivityType_ = false;

template <typename T, HolderType Holder>
constexpr inline auto
    LinkedListConnectivityType_<LinkedListConnectivity<T, Holder>> = true;

} // namespace detail

template <typename T>
concept LinkedListConnectivityType = requires
{
    typename T::connectivity_type;
    detail::LinkedListConnectivityType_<typename T::connectivity_type>;
};

template <typename T>
using ValueLinkedListConnectivity = LinkedListConnectivity<T, ValueHolder<T>>;

template <typename T>
using ValueLinkedListItem = ValueLinkedListConnectivity<T>;

template <typename T>
using IntrusiveLinkedListConnectivity = LinkedListConnectivity<T, BaseOf<T>>;

template <LinkedListConnectivityType Node>
requires std::is_default_constructible_v<Node>
class IntrusiveLinkedList final
{
public:
    using value_type = Node::value_type;

    template <typename ValueType>
    class Iterator final
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = ValueType;
        using difference_type = std::ptrdiff_t;
        using index_type = size_t;
        using pointer = ValueType*;
        using reference = ValueType&;

        Iterator() = default;

        explicit Iterator(ValueType& node) :
            node_{&node}
        {}

        reference operator*() const { return *node_; }
        pointer operator->() const { return node_; }

        Iterator& operator++()
        {
            node_ = &node_->next();
            return *this;
        }

        Iterator& operator--() {
            node_ = &node_->prev();
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

        friend bool operator==(const Iterator& a, const Iterator& b)
        {
            return a.node_ == b.node_;
        }

    private:
        ValueType* node_{};
    };
    using iterator = Iterator<typename Node::connectivity_type>;
    using const_iterator = Iterator<const typename Node::connectivity_type>;

    auto link(Node& node) -> void
    { node.link_to(root_); }

    auto begin() -> iterator
    { return iterator{ root_.next() }; }

    auto begin() const -> const_iterator
    { return const_iterator{ root_.next() }; }

    auto end() -> iterator
    { return iterator{ root_ }; }

    auto end() const -> const_iterator
    { return const_iterator{ root_ }; }

private:
    Node root_;
};

template <typename T>
using ValueLinkedList = IntrusiveLinkedList<ValueLinkedListItem<T>>;

} // namespace mpk::mix
