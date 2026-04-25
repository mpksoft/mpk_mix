/** @file
 * @brief Dot-separated field path for navigating nested serial structures
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/util/any_of.hpp"
#include "mpk/mix/util/format_streamable.hpp"

#include <ostream>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace mpk::mix {

struct PathItem final
{
public:
    PathItem();

    /* implicit */ PathItem(size_t index);
    /* implicit */ PathItem(std::string_view name);
    /* implicit */ PathItem(std::string name);

    auto is_index() const noexcept -> bool;
    auto is_name() const noexcept -> bool;
    auto index() const -> size_t;
    auto name() const -> std::string_view;

    friend auto operator<<(std::ostream& s, PathItem const& item)
        -> std::ostream&;

    auto operator==(PathItem const&) const noexcept -> bool;

    static auto from_string(std::string_view s) -> PathItem;

private:
    using Storage = std::variant<size_t, std::string_view, std::string>;
    Storage storage_;
};

using PathView = std::span<PathItem const>;

auto operator<<(std::ostream& s, PathView path) -> std::ostream&;

class Path;

template <typename T>
concept PathLikeType = AnyOf<T, PathView, Path>;

class Path : public std::vector<PathItem>
{
public:
    using Base = std::vector<PathItem>;
    using Base::Base;

    inline auto operator/=(PathItem const& i2) -> Path&;

    template <PathLikeType PathLike>
    inline auto operator/=(PathLike const& p2) -> Path&;

    [[nodiscard]] auto view() const& noexcept -> PathView;

    friend auto operator<<(std::ostream& s, Path const& path) -> std::ostream&;

    static auto from_string(std::string_view s) -> Path;
    static auto from_view(PathView view) -> Path;
};

inline auto operator/(PathItem i1, PathItem i2) -> Path
{
    return Path{i1, i2};
}

template <PathLikeType PathLike>
inline auto operator/(PathLike const& p1, PathItem i2) -> Path
{
    auto result = Path{p1};
    result.push_back(i2);
    return result;
}

template <PathLikeType PathLike>
inline auto operator/(PathItem const& i1, PathLike const& p2) -> Path
{
    auto result = Path{i1};
    result.insert(result.end(), p2.begin(), p2.end());
    return result;
}

template <PathLikeType PathLike1, PathLikeType PathLike2>
inline auto operator/(PathLike1 const& p1, PathLike2 const& p2) -> Path
{
    auto result = Path{p1};
    result.insert(result.end(), p2.begin(), p2.end());
    return result;
}

inline auto Path::operator/=(PathItem const& i2) -> Path&
{
    return *this = *this / i2;
}

template <PathLikeType PathLike>
inline auto Path::operator/=(PathLike const& p2) -> Path&
{
    return *this = *this / p2;
}

class ScopedPathAppender final
{
public:
    ScopedPathAppender(Path& path, PathItem item) : path_{path}
    {
        path_ /= std::move(item);
    }

    ~ScopedPathAppender()
    {
        path_.pop_back();
    }

    auto path() const -> PathView
    {
        return path_;
    }

private:
    Path& path_;
};

} // namespace mpk::mix

MPKMIX_DECL_OSTREAM_FORMATTER(mpk::mix::PathItem);
MPKMIX_DECL_OSTREAM_FORMATTER(mpk::mix::Path);
MPKMIX_DECL_OSTREAM_FORMATTER(mpk::mix::PathView);
