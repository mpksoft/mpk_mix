/** @file
 * @brief Implementation of IPv4 address lookup for network interfaces
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/wrap_c/get_ipv4_addr_of.hpp"

#include "mpk/mix/util/throw.hpp"
#include "mpk/mix/wrap_c/zero_filled.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <regex>
#include <string>

namespace mpk::mix::wrap_c {

namespace {

class ScopedAddrInfo final
{
public:
    ~ScopedAddrInfo()
    {
        if (ai)
            freeaddrinfo(ai);
    }

    explicit ScopedAddrInfo(struct addrinfo* ai)
        : ai(ai)
    {}

    ScopedAddrInfo(const ScopedAddrInfo&) = delete;
    auto operator=(const ScopedAddrInfo&) -> ScopedAddrInfo& = delete;

private:
    struct addrinfo* ai;
};

} // anonymous namespace

auto get_ipv4_addr_of(const std::string& host) -> std::string
{
    static const auto rx_ip =
        std::regex("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$");

    if (std::regex_match(host, rx_ip))
        return host;

    auto hints = zero_filled<struct addrinfo>();
    hints.ai_family = AF_INET;
    hints.ai_socktype = 0;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    struct addrinfo* result = nullptr;
    if (auto s = getaddrinfo(host.c_str(), nullptr, &hints, &result); s != 0)
        throw_<std::invalid_argument>("getaddrinfo: {}", gai_strerror(s));

    auto scoped_addr_info = ScopedAddrInfo{result};

    for (auto rp = result; rp; rp = rp->ai_next)
    {
        if (rp->ai_family != AF_INET)
            continue;

        const auto& sock_iaddr = *reinterpret_cast<sockaddr_in*>(rp->ai_addr);
        auto& addr = sock_iaddr.sin_addr.s_addr;
        char* ip = inet_ntoa(*reinterpret_cast<const in_addr*>(&addr));
        return ip;
    }

    return host;
}

} // namespace mpk::mix::wrap_c
