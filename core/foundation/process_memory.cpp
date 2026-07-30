/**
 * @file process_memory.cpp
 */

#include "process_memory.hpp"

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#elif defined(__APPLE__)
#include <mach/mach.h>
#else
#include <cstdio>
#include <cstring>
#endif

namespace scope::foundation
{

std::optional<ProcessMemoryUsage> currentProcessMemoryUsage()
{
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS counters{};

    if (!GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)))
    {
        return std::nullopt;
    }

    ProcessMemoryUsage usage;
    usage.residentBytes = static_cast<std::uint64_t>(counters.WorkingSetSize);

    return usage;
#elif defined(__APPLE__)
    mach_task_basic_info info{};
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;

    const kern_return_t status =
        task_info(mach_task_self(), MACH_TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&info), &count);

    if (status != KERN_SUCCESS)
    {
        return std::nullopt;
    }

    ProcessMemoryUsage usage;
    usage.residentBytes = static_cast<std::uint64_t>(info.resident_size);

    return usage;
#else
    std::FILE* stream = std::fopen("/proc/self/status", "r");

    if (stream == nullptr)
    {
        return std::nullopt;
    }

    char line[256];
    ProcessMemoryUsage usage;

    while (std::fgets(line, sizeof(line), stream) != nullptr)
    {
        if (std::strncmp(line, "VmRSS:", 6) != 0)
        {
            continue;
        }

        unsigned long kilobytes = 0U;

        if (std::sscanf(line + 6, "%lu", &kilobytes) != 1)
        {
            std::fclose(stream);

            return std::nullopt;
        }

        usage.residentBytes = static_cast<std::uint64_t>(kilobytes) * 1024U;
        std::fclose(stream);

        return usage;
    }

    std::fclose(stream);

    return std::nullopt;
#endif
}

} // namespace scope::foundation
