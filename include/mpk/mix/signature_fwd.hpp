/** @file
 * @brief Forward declarations for signature type traits
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

namespace mpk::mix
{

template <typename S>
constexpr inline auto is_signature_type = false;

template <typename R, typename... Args, bool NX>
constexpr inline auto is_signature_type<R(Args...) noexcept(NX)> = true;

template <typename S>
concept SignatureType = is_signature_type<S>;

} // namespace mpk::mix
