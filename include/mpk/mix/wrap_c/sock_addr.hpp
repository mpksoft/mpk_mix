/** @file
 * @brief SockAddrLite lightweight socket address value type
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/log/defs.hpp"
#include "mpk/mix/wrap_c/host_addr.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>

namespace mpk::mix::wrap_c {

struct SockAddr
{
    HostAddr host;
    int port{};

    auto empty() const noexcept { return host.empty(); }

    operator bool() const noexcept { return host; }
};

auto operator<<(std::ostream&, const SockAddr&) -> std::ostream&;

#if MPKMIX_LOG_ENGINE == MPKMIX_LOG_QUILL

auto format_as(SockAddr c) -> std::string;

#endif // MPKMIX_LOG_ENGINE == MPKMIX_LOG_QUILL


struct SockAddrLite
{
    uint32_t host{};
    int port{};

    SockAddrLite() = default;

    SockAddrLite(const SockAddr& a)
        : host{a.host}
        , port{a.port}
    {}

    SockAddrLite(uint32_t host, int port)
        : host{host}
        , port{port}
    {}

    auto empty() const noexcept { return host == 0; }

    operator bool() const noexcept { return host != 0; }

    auto sock_addr() const -> SockAddr { return {.host = host, .port = port}; }

    friend constexpr auto operator<=>(const SockAddrLite&, const SockAddrLite&) noexcept
        -> std::strong_ordering = default;

    auto hash() const noexcept -> size_t
    {
        return std::hash<uint32_t>{}(host) ^ std::hash<int>{}(port);
    }

    struct Hash
    {
        auto operator()(const SockAddrLite& a) const noexcept -> size_t
        {
            return a.hash();
        }
    };
};

auto operator<<(std::ostream&, const SockAddrLite&) -> std::ostream&;


#if MPKMIX_LOG_ENGINE == MPKMIX_LOG_QUILL

auto format_as(SockAddrLite c) -> std::string;

#endif // MPKMIX_LOG_ENGINE == MPKMIX_LOG_QUILL

} // namespace mpk::mix::wrap_c


#if MPKMIX_LOG_ENGINE == MPKMIX_LOG_QUILL

#include "quill/DeferredFormatCodec.h"

template <>
struct fmtquill::formatter<mpk::mix::wrap_c::SockAddrLite>
{
    constexpr auto parse(fmtquill::format_parse_context& ctx) { return ctx.begin(); }

    auto format(const mpk::mix::wrap_c::SockAddrLite& c,
                fmtquill::format_context& ctx) const -> decltype(ctx.out())
    {
        auto s = mpk::mix::wrap_c::format_as(c);
        return fmtquill::format_to(
            ctx.out(), "{}", fmtquill::string_view{s.data(), s.size()});
    }
};

template <>
struct quill::Codec<mpk::mix::wrap_c::SockAddrLite>
    : quill::DeferredFormatCodec<mpk::mix::wrap_c::SockAddrLite>
{};

#endif // MPKMIX_LOG_ENGINE == MPKMIX_LOG_QUILL
