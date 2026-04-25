/** @file
 * @brief Parse a YAML node into a Value object
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/value/type_fwd.hpp"
#include "mpk/mix/value/value_fwd.hpp"
#include "mpk/mix/value/value_registry.hpp"

#include <yaml-cpp/node/node.h>

namespace mpk::mix::serial::yaml {

auto parse_value(
    const YAML::Node& node,
    const value::Type* type,
    const value::ValueRegistry<const value::Type*>& type_registry) -> value::Value;

auto parse_value(
    const YAML::Node& node,
    const value::ValueRegistry<const value::Type*>& type_registry)
    -> value::Value;

} // namespace mpk::mix::serial::yaml
