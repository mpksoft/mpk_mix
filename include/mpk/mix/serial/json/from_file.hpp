/** @file
 * @brief Load and parse a JSON document from a file path
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <nlohmann/json_fwd.hpp>

#include <filesystem>

namespace mpk::mix::serial::json {

auto from_file(std::filesystem::path const& path) -> nlohmann::json;

auto array_from_file(std::filesystem::path const& path) -> nlohmann::json;

} // namespace mpk::mix::serial::json
