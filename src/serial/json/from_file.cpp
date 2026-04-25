/** @file
 * @brief Implementation of JSON file loader
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#include "mpk/mix/serial/json/from_file.hpp"

#include "mpk/mix/util/throw.hpp"

#include <nlohmann/json.hpp>

#include <fstream>

namespace mpk::mix::serial::json {

auto from_file(std::filesystem::path const& path) -> nlohmann::json
{
    std::ifstream in(path);
    if (!in.is_open())
        throw_("Unable to open file '{}'", path.string());

    nlohmann::json result;
    try
    {
        in >> result;
    }
    catch (nlohmann::json::parse_error const& e)
    {
        throw_(
            "JSON parse error in file '{}' at byte {}: {}",
            path.string(),
            e.byte,
            e.what());
    }
    return result;
}

auto array_from_file(std::filesystem::path const& path) -> nlohmann::json
{
    auto result = from_file(path);
    if (!result.is_array())
        throw_(
            "Expected JSON array in file '{}', but found {}",
            path.string(),
            result.type_name());
    return result;
}

} // namespace mpk::mix::serial::json
