/** @file
 * @brief ValuePath, ValuePathItem — implementation.
 *
 * Copyright (C) 2024-2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/value/value_path.hpp"

#include "mpk/mix/util/format_seq.hpp"
#include "mpk/mix/util/throw.hpp"

#include <charconv>


namespace mpk::mix::value {

namespace {

template <ValuePathLikeType ValuePathLike>
auto print_path(std::ostream& s, const ValuePathLike& path)
    -> void
{ s << '/' << mpk::mix::format_seq(path, "/"); }

} // anonymous namespace


ValuePathItem::ValuePathItem() = default;

ValuePathItem::ValuePathItem(size_t index) :
    storage_{ index }
{}

ValuePathItem::ValuePathItem(std::string_view name) :
    storage_{ name }
{}

ValuePathItem::ValuePathItem(std::string name) :
    storage_{ std::move(name) }
{}

auto ValuePathItem::is_index() const noexcept
    -> bool
{ return holds_alternative<size_t>(storage_); }

auto ValuePathItem::is_name() const noexcept
    -> bool
{ return !is_index(); }

auto ValuePathItem::index() const
    -> size_t
{
    if (!is_index())
        throw std::invalid_argument(
            "Retrieving index from value path item which is a name");
    return std::get<size_t>(storage_);
}

auto ValuePathItem::name() const
    -> std::string_view
{
    if (!is_name())
        throw std::invalid_argument(
            "Retrieving name from value path item which is an index");
    if (holds_alternative<std::string_view>(storage_))
        return std::get<std::string_view>(storage_);
    return std::get<std::string>(storage_);
}

auto operator<<(std::ostream& s, const ValuePathItem& item)
    -> std::ostream&
{
    std::visit([&](const auto& typed){ s << typed; }, item.storage_);
    return s;
}

auto ValuePathItem::operator==(const ValuePathItem&) const noexcept
    -> bool = default;

auto ValuePathItem::from_string(std::string_view s)
    -> ValuePathItem
{
    size_t index{};
    auto fcres = std::from_chars(s.begin(), s.end(), index);
    if (fcres.ec != std::error_code{} || fcres.ptr != s.end())
        return { std::string{ s } };
    else
        return { index };
}


auto operator<<(std::ostream& s, ValuePathView path)
    -> std::ostream&
{
    print_path(s, path);
    return s;
}

auto operator<<(std::ostream& s, const ValuePath& path)
    -> std::ostream&
{
    print_path(s, path);
    return s;
}

auto ValuePath::from_string(std::string_view s)
    -> ValuePath
{
    if (s.empty() || s[0] != '/')
        mpk::mix::throw_<std::invalid_argument>(
            "Failed to parse ValuePath from string '", s,
            "' - must start with '/'");

    auto result = ValuePath{};

    auto pos = std::string::size_type{0};
    while (pos != std::string::npos)
    {
        auto next_pos = s.find_first_of('/', pos+1);
        auto item_s =
            next_pos == std::string::npos
                ? s.substr(pos+1)
                : s.substr(pos+1, next_pos - pos - 1);
        result.push_back(ValuePathItem::from_string(item_s));
        pos = next_pos;
    }

    return result;
}

} // namespace mpk::mix::value
