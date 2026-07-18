/**
 * @file log_macros.hpp
 * @brief Convenience macros for diagnostic logging.
 */

#pragma once

#include "diagnostics.hpp"

#define SCOPE_LOG_DEBUG(category, message)                                                                             \
    ::scope::runtime::Diagnostics::instance().log(::scope::runtime::LogLevel::Debug, category, message)

#define SCOPE_LOG_INFO(category, message)                                                                              \
    ::scope::runtime::Diagnostics::instance().log(::scope::runtime::LogLevel::Info, category, message)

#define SCOPE_LOG_WARN(category, message)                                                                              \
    ::scope::runtime::Diagnostics::instance().log(::scope::runtime::LogLevel::Warning, category, message)

#define SCOPE_LOG_ERROR(category, message)                                                                             \
    ::scope::runtime::Diagnostics::instance().log(::scope::runtime::LogLevel::Error, category, message)
