/** @file
 * @brief Registry of factory functions for polymorphic objects
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/value/value_registry.hpp"

#include <memory>

namespace mpk::mix::value
{

template <typename Interface, typename... ConstructionArgs>
using FactoryFunc = std::shared_ptr<Interface>(*)(ConstructionArgs...);

template <typename Interface, typename... ConstructionArgs>
using ObjectRegistry =
    ValueRegistry<FactoryFunc<Interface, ConstructionArgs...>>;

} // namespace mpk::mix::value
