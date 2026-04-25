/** @file
 * @brief Log level constants and type definitions
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include "mpk/mix/log/defs.hpp"

#if MPKMIX_LOG_ENGINE == MPKMIX_LOG_QUILL

#include "quill/core/LogLevel.h"

namespace mpk::mix {
using quill::LogLevel;
} // namespace mpk::mix

#else

#include <cstdint>

namespace mpk::mix {

enum class LogLevel : uint8_t
{
    TraceL3,
    TraceL2,
    TraceL1,
    Debug,
    Info,
    Warning,
    Error,
    Critical,
    None
};

} // namespace mpk::mix

#endif
