/** @file
 * @brief Implementation of temporary scratch directory
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/util/scratch_dir.hpp"

#include "mpk/mix/util/throw.hpp"

#include <cassert>
#include <format>
#include <iostream>
#include <random>

namespace mpk::mix
{

namespace fs = std::filesystem;

ScratchDir::ScratchDir(std::string_view name_prefix)
    : ScratchDir{{}, name_prefix}
{}

ScratchDir::ScratchDir(fs::path const& parent_dir, std::string_view name_prefix)
{
    if (name_prefix.empty())
        name_prefix = "temp_dir";

    fs::path temp_dir =
        parent_dir.empty() ? fs::temp_directory_path() : parent_dir;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> dis(0, 65535);

    for (int i = 0; i < 10; ++i)
    {
        auto subdir_name = std::format("{}_{:04x}", name_prefix, dis(gen));
        fs::path unique_dir = temp_dir / subdir_name;
        if (fs::create_directories(unique_dir))
        {
            path_ = std::move(unique_dir);
            assert(!path_.empty());
            return;
        }
    }
    throw_("Failed to create a unique temporary directory after several attempts.");
}

ScratchDir::~ScratchDir()
{
    if (empty())
        return;
    try
    {
        fs::remove_all(path_);
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error removing directory: " << e.what() << std::endl;
    }
}

auto ScratchDir::empty() const noexcept -> bool
{
    return path_.empty();
}

auto ScratchDir::detach() noexcept -> void
{
    path_.clear();
}

auto ScratchDir::path() const noexcept -> fs::path const&
{
    return path_;
}

ScratchDir::operator fs::path const&() const noexcept
{
    return path_;
}

} // namespace mpk::mix
