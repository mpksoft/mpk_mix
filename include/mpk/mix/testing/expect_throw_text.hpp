/** @file
 * @brief EXPECT_THROW_TEXT helper that asserts exception message content
 *
 * Copyright (C) 2026 MPK Software, St.-Petersburg, Russia
 */

#pragma once

#include <gtest/gtest.h>

#include <regex>
#include <stdexcept>
#include <string>

// cSpell:disable

#define MPKMIX_EXPECT_THROW_MESSAGE(statement, ExceptionType, expected_message) \
    try                                                                          \
    {                                                                            \
        statement;                                                               \
        FAIL()                                                                   \
            << "Expected exception of type " #ExceptionType                     \
               " with message \"" << expected_message                            \
            << "\" but no exception was thrown.";                                \
    }                                                                            \
    catch (const ExceptionType& e)                                               \
    {                                                                            \
        EXPECT_STREQ(expected_message, e.what())                                 \
            << "Exception message mismatch. Expected: \"" << expected_message    \
            << "\", Got: \"" << e.what() << "\".";                               \
    }                                                                            \
    catch (const std::exception& e)                                              \
    {                                                                            \
        FAIL() << "Caught unexpected exception type. Expected "                  \
               #ExceptionType                                                    \
               << ", but got std::exception with message: " << e.what();        \
    }                                                                            \
    catch (...)                                                                  \
    {                                                                            \
        FAIL() << "Caught an unknown exception type. Expected "                  \
               #ExceptionType << ".";                                            \
    }

#define MPKMIX_EXPECT_THROW_REGEX(statement, ExceptionType, expected_regex_str) \
    try                                                                          \
    {                                                                            \
        statement;                                                               \
        FAIL() << "Expected exception of type " #ExceptionType                  \
                  " with message matching regex \""                              \
               << expected_regex_str << "\" but no exception was thrown.";      \
    }                                                                            \
    catch (const ExceptionType& e)                                               \
    {                                                                            \
        const std::string actual_message = e.what();                             \
        const std::regex regex_pattern(expected_regex_str);                      \
        if (!std::regex_match(actual_message, regex_pattern))                    \
        {                                                                        \
            FAIL() << "Exception message mismatch. Expected regex: \""           \
                   << expected_regex_str << "\", Got: \"" << actual_message      \
                   << "\".";                                                     \
        }                                                                        \
    }                                                                            \
    catch (const std::exception& e)                                              \
    {                                                                            \
        FAIL() << "Caught unexpected exception type. Expected "                  \
               #ExceptionType                                                    \
               << ", but got std::exception with message: " << e.what();        \
    }                                                                            \
    catch (...)                                                                  \
    {                                                                            \
        FAIL() << "Caught an unknown exception type. Expected "                  \
               #ExceptionType << ".";                                            \
    }

// cSpell:enable
