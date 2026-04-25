/** @file
 * @brief Host address representation and resolution utilities
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>

namespace mpk::mix::wrap_c {

class HostAddr
{
public:
    HostAddr() = default;
    /* implicit */ HostAddr(std::string host_or_ipv4_addr);
    /* implicit */ HostAddr(std::string_view host_or_ipv4_addr);
    /* implicit */ HostAddr(uint32_t ipv4_addr) noexcept;

    auto empty() const noexcept -> bool;

    operator bool() const noexcept;

    auto as_string() const -> const std::string&;
    auto as_uint32() const -> uint32_t;

    operator const std::string&() const;
    operator uint32_t() const;

private:
    mutable std::string addr_str_;
    mutable uint32_t addr_{};
};

auto operator<<(std::ostream&, const HostAddr&) -> std::ostream&;

} // namespace mpk::mix::wrap_c
