/**
 * @file process_memory.hpp
 * @brief Current process memory usage snapshot.
 */

#pragma once

#include <cstdint>
#include <optional>

namespace scope::foundation
{

/**
 * @brief Resident memory for the current process.
 */
struct ProcessMemoryUsage
{
    std::uint64_t residentBytes{0U};
};

/**
 * @brief Returns RSS for the current process when supported on the host OS.
 */
[[nodiscard]] std::optional<ProcessMemoryUsage> currentProcessMemoryUsage();

} // namespace scope::foundation
