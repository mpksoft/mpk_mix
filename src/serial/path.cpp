/** @file
 * @brief Implementation of dot-separated serialization path
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/serial/path.hpp"

#include "mpk/mix/util/format_seq.hpp"
#include "mpk/mix/util/throw.hpp"

#include <charconv>

namespace mpk::mix {

namespace {

template <PathLikeType PathLike>
auto print_path(std::ostream& s, PathLike const& path) -> void
{
    s << '/' << format_seq(path, "/");
}

} // anonymous namespace

PathItem::PathItem() = default;

PathItem::PathItem(size_t index) : storage_{index} {}

PathItem::PathItem(std::string_view name) : storage_{name} {}

PathItem::PathItem(std::string name) : storage_{std::move(name)} {}

auto PathItem::is_index() const noexcept -> bool
{
    return holds_alternative<size_t>(storage_);
}

auto PathItem::is_name() const noexcept -> bool
{
    return !is_index();
}

auto PathItem::index() const -> size_t
{
    if (!is_index())
        throw std::invalid_argument(
            "Retrieving index from path item which is a name");
    return std::get<size_t>(storage_);
}

auto PathItem::name() const -> std::string_view
{
    if (!is_name())
        throw std::invalid_argument(
            "Retrieving name from path item which is an index");
    if (holds_alternative<std::string_view>(storage_))
        return std::get<std::string_view>(storage_);
    return std::get<std::string>(storage_);
}

auto operator<<(std::ostream& s, PathItem const& item) -> std::ostream&
{
    std::visit([&](auto const& v) { s << v; }, item.storage_);
    return s;
}

auto PathItem::operator==(PathItem const& that) const noexcept -> bool
{
    if (is_index())
        return that.is_index() ? index() == that.index() : false;
    else
        return that.is_name() ? name() == that.name() : false;
}

auto PathItem::from_string(std::string_view s) -> PathItem
{
    size_t index{};
    auto [ptr, ec] = std::from_chars(s.begin(), s.end(), index);
    if (ec != std::error_code{} || ptr != s.end())
        return {std::string{s}};
    return {index};
}

auto operator<<(std::ostream& s, PathView path) -> std::ostream&
{
    print_path(s, path);
    return s;
}

auto operator<<(std::ostream& s, Path const& path) -> std::ostream&
{
    print_path(s, path);
    return s;
}

auto Path::view() const& noexcept -> PathView
{
    return *this;
}

auto Path::from_string(std::string_view s) -> Path
{
    if (s.empty() || s[0] != '/')
        throw_<std::invalid_argument>(
            "Failed to parse Path from string '{}' - must start with '/'", s);

    auto result = Path{};
    auto pos = std::string::size_type{0};
    while (pos != std::string::npos)
    {
        auto next_pos = s.find_first_of('/', pos + 1);
        auto item_s = next_pos == std::string::npos
                          ? s.substr(pos + 1)
                          : s.substr(pos + 1, next_pos - pos - 1);
        result.push_back(PathItem::from_string(item_s));
        pos = next_pos;
    }
    return result;
}

auto Path::from_view(PathView view) -> Path
{
    return Path(view.begin(), view.end());
}

} // namespace mpk::mix
