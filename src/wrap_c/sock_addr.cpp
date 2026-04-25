/** @file
 * @brief Implementation of SockAddrLite socket address utilities
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/wrap_c/sock_addr.hpp"

#include "mpk/mix/util/format_sep.hpp"

namespace mpk::mix::wrap_c {

auto operator<<(std::ostream& s, const SockAddr& addr) -> std::ostream&
{
    return s << addr.host << ':' << addr.port;
}

#if MPKMIX_LOG_ENGINE == MPKMIX_LOG_QUILL

auto format_as(SockAddr c) -> std::string
{
    return std::format("{}:{}", format_as(c.host), c.port);
}

#endif // MPKMIX_LOG_ENGINE == MPKMIX_LOG_QUILL

auto operator<<(std::ostream& s, const SockAddrLite& addr) -> std::ostream&
{
    return s << SockAddr{HostAddr{addr.host}, addr.port};
}


#if MPKMIX_LOG_ENGINE == MPKMIX_LOG_QUILL

auto format_as(SockAddrLite c) -> std::string
{
    return format_sep("", c);
}

#endif // MPKMIX_LOG_ENGINE == MPKMIX_LOG_QUILL

} // namespace mpk::mix::wrap_c
