/** @file
 * @brief Log engine selector macros (MPKMIX_LOG_NONE, MPKMIX_LOG_QUILL, MPKMIX_LOG_CERR)
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#define MPKMIX_LOG_NONE 0
#define MPKMIX_LOG_QUILL 1
#define MPKMIX_LOG_CERR 2

#ifndef MPKMIX_LOG_ENGINE
#define MPKMIX_LOG_ENGINE MPKMIX_LOG_QUILL
#endif
