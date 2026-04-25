/** @file
 * @brief FixedLengthString<N> fixed-capacity null-terminated string
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <cassert>
#include <cstring>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace mpk::mix
{

template <typename CharT, size_t N, typename Traits = std::char_traits<CharT>>
class BasicFixedLengthString;

namespace detail
{

template <typename T>
constexpr inline auto is_fixed_length_string = false;

template <typename CharT, size_t N, typename Traits>
constexpr inline auto
    is_fixed_length_string<BasicFixedLengthString<CharT, N, Traits>> = true;

template <typename T>
concept fixed_length_string_type = is_fixed_length_string<T>;

template <typename T, typename CharT, typename Traits>
constexpr inline auto is_compatible_string = false;

template <typename CharT, typename Traits, typename Alloc>
constexpr inline auto
    is_compatible_string<std::basic_string<CharT, Traits, Alloc>, CharT, Traits> =
        true;

template <typename CharT, typename Traits>
constexpr inline auto
    is_compatible_string<std::basic_string_view<CharT, Traits>, CharT, Traits> =
        true;

template <typename CharT>
constexpr inline auto
    is_compatible_string<CharT const*, CharT, std::char_traits<CharT>> = true;

template <typename CharT, size_t N>
constexpr inline auto
    is_compatible_string<CharT[N], CharT, std::char_traits<CharT>> = true;

template <typename CharT, size_t N>
constexpr inline auto
    is_compatible_string<CharT const[N], CharT, std::char_traits<CharT>> = true;

template <typename T, typename CharT, typename Traits>
concept CompatibleString = is_compatible_string<T, CharT, Traits>;

} // namespace detail

template <typename CharT, size_t N, typename Traits>
class BasicFixedLengthString final
{
public:
    template <typename Alloc>
    using CompatibleString = std::basic_string<CharT, Traits, Alloc>;

    using DefaultCompatibleString = CompatibleString<std::allocator<CharT>>;

    using CompatibleStringView = std::basic_string_view<CharT, Traits>;

    static constexpr auto capacity = N;

    BasicFixedLengthString() = default;

    BasicFixedLengthString(BasicFixedLengthString const&) = default;

    auto operator=(BasicFixedLengthString const&)
        -> BasicFixedLengthString& = default;

    /* implicit */ BasicFixedLengthString(CharT const* s, size_t size) :
      size_(size)
    {
        if (size_ > N)
        {
            std::ostringstream out;
            out << "Initializer '" << CompatibleStringView{s, size_}
                << "' (length " << size << ") for fixed-length string"
                << " (capacity " << N << ") is too long";
            throw std::invalid_argument(out.str());
        }
        memcpy(data_, s, size_);
    }

    // Note: We assume here that s is zero-terminated
    template <size_t N2>
    /* implicit */ BasicFixedLengthString(CharT const (&s)[N2]) :
      BasicFixedLengthString(s, N2 - 1)
    {
        static_assert(N2 > 0);
        assert(s[N2 - 1] == 0);
        static_assert(
            N2 <= N + 1, "Fixed-length string initializer is too long");
    }

    template <typename Alloc>
    /* implicit */ BasicFixedLengthString(CompatibleString<Alloc> const& s) :
      BasicFixedLengthString(s.data(), s.size())
    {
    }

    /* implicit */ BasicFixedLengthString(CompatibleStringView s) :
      BasicFixedLengthString(s.data(), s.size())
    {
    }

    template <size_t N2>
    /* implicit */ BasicFixedLengthString(
        BasicFixedLengthString<CharT, N2, Traits> const& s) :
      BasicFixedLengthString(s.view())
    {
    }

    auto size() const noexcept -> size_t
    {
        return size_;
    }

    auto empty() const noexcept -> bool
    {
        return size_ == 0;
    }

    auto view() const noexcept -> std::basic_string_view<CharT, Traits>
    {
        return {data_, size_};
    }

    auto string() const noexcept -> DefaultCompatibleString
    {
        return DefaultCompatibleString(data_, size_);
    }

    explicit operator DefaultCompatibleString() const noexcept
    {
        return string();
    }

    operator CompatibleStringView() const noexcept
    {
        return view();
    }

    friend auto operator<<(std::ostream& out, BasicFixedLengthString const& s)
        -> std::ostream&
    {
        return out << s.view();
    }

    auto operator<=>(BasicFixedLengthString const& that) const noexcept
        -> std::strong_ordering
    {
        return view() <=> that.view();
    }

    auto operator==(BasicFixedLengthString const& that) const noexcept -> bool
    {
        return view() == that.view();
    }

    auto operator!=(BasicFixedLengthString const& that) const noexcept -> bool
    {
        return view() != that.view();
    }

    template <detail::CompatibleString<CharT, Traits> T>
    friend auto operator<=>(BasicFixedLengthString const& lhs, T const& rhs)
        -> std::strong_ordering
    {
        return lhs.view() <=> std::string_view{rhs};
    }

    template <detail::CompatibleString<CharT, Traits> T>
    friend auto operator<=>(T const& lhs, BasicFixedLengthString const& rhs)
        -> std::strong_ordering
    {
        return std::string_view{lhs} <=> rhs.view();
    }

    template <detail::CompatibleString<CharT, Traits> T>
    friend auto operator==(BasicFixedLengthString const& lhs, T const& rhs)
        -> bool
    {
        return lhs.view() == std::string_view{rhs};
    }

    template <detail::CompatibleString<CharT, Traits> T>
    friend auto operator==(T const& lhs, BasicFixedLengthString const& rhs)
        -> bool
    {
        return rhs == lhs;
    }

    template <detail::CompatibleString<CharT, Traits> T>
    friend auto operator!=(BasicFixedLengthString const& lhs, T const& rhs)
        -> bool
    {
        return lhs.view() != std::string_view{rhs};
    }

    template <detail::CompatibleString<CharT, Traits> T>
    friend auto operator!=(T const& lhs, BasicFixedLengthString const& rhs)
        -> bool
    {
        return rhs != lhs;
    }

private:
    CharT data_[N];
    size_t size_{};
};

template <size_t N>
using FixedLengthString = BasicFixedLengthString<char, N>;

} // namespace mpk::mix
