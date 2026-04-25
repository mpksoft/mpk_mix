/** @file
 * @brief RAII file-descriptor wrapper with move semantics
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/func_ref/tags.hpp"

namespace mpk::mix::wrap_c {

class Fd final
{
public:
    ~Fd();

    Fd() noexcept;

    Fd(Unsafe_Tag, int fd) noexcept;

    Fd(Fd&& that) noexcept;

    auto operator=(Fd&& that) noexcept -> Fd&;

    Fd(Fd const&) noexcept = delete;

    auto operator=(Fd const& that) noexcept -> Fd& = delete;

    [[nodiscard]] auto empty() const noexcept -> bool;

    [[nodiscard]] auto fd() const noexcept -> int;

    [[nodiscard]] auto detach(Unsafe_Tag) noexcept -> int;

    auto close() -> void;

    auto dup() const -> Fd;

private:
    int fd_{};
};

} // namespace mpk::mix::wrap_c
