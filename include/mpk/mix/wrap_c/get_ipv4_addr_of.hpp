/** @file
 * @brief Look up the IPv4 address of a named network interface
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <string>

namespace mpk::mix::wrap_c {

auto get_ipv4_addr_of(const std::string& host) -> std::string;

} // namespace mpk::mix::wrap_c
