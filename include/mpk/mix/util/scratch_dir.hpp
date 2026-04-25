/** @file
 * @brief Temporary scratch directory created on construction, removed on destruction
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <filesystem>
#include <string_view>

namespace mpk::mix
{

class ScratchDir final
{
public:
    explicit ScratchDir(std::string_view name_prefix = {});

    explicit ScratchDir(
        std::filesystem::path const& parent_dir,
        std::string_view name_prefix = {});

    ScratchDir(ScratchDir const&) = delete;
    ScratchDir(ScratchDir&&) = default;
    auto operator=(ScratchDir const&) -> ScratchDir& = delete;
    auto operator=(ScratchDir&&) -> ScratchDir& = default;

    ~ScratchDir();

    auto empty() const noexcept -> bool;

    auto detach() noexcept -> void;

    auto path() const noexcept -> std::filesystem::path const&;

    operator std::filesystem::path const&() const noexcept;

private:
    std::filesystem::path path_;
};

} // namespace mpk::mix
