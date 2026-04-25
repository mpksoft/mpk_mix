/** @file
 * @brief Implementation of host address resolution
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/wrap_c/check_c_call.hpp"
#include "mpk/mix/wrap_c/get_ipv4_addr_of.hpp"
#include "mpk/mix/wrap_c/host_addr.hpp"

#include "mpk/mix/util/throw.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

namespace mpk::mix::wrap_c {

HostAddr::HostAddr(std::string host_or_ipv4_addr)
    : addr_str_{get_ipv4_addr_of(host_or_ipv4_addr)}
{}

HostAddr::HostAddr(std::string_view host_or_ipv4_addr)
    : HostAddr{std::string{host_or_ipv4_addr}}
{}

HostAddr::HostAddr(uint32_t ipv4_addr) noexcept
    : addr_{ipv4_addr}
{}

auto HostAddr::empty() const noexcept -> bool
{
    return addr_ == 0 && addr_str_.empty();
}

HostAddr::operator bool() const noexcept
{
    return !empty();
}

auto HostAddr::as_string() const -> const std::string&
{
    if (addr_ != 0 && addr_str_.empty())
    {
        auto addr_str = inet_ntoa(in_addr{addr_});
        if (!addr_str)
            throw_<std::invalid_argument>("Failed to retrieve addr");
        addr_str_ = addr_str;
    }
    return addr_str_;
}

auto HostAddr::as_uint32() const -> uint32_t
{
    if (addr_ == 0 && !addr_str_.empty())
    {
        auto in = in_addr{};
        MPKMIX_THROW_IF_ZERO(inet_aton(addr_str_.c_str(), &in), "Bad IP address");
        addr_ = in.s_addr;
    }
    return addr_;
}

HostAddr::operator const std::string&() const
{
    return as_string();
}

HostAddr::operator uint32_t() const
{
    return as_uint32();
}

auto operator<<(std::ostream& s, const HostAddr& addr) -> std::ostream&
{
    return s << addr.as_string();
}

} // namespace mpk::mix::wrap_c
