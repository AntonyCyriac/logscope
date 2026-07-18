/**
 * @file process.hpp
 * @brief Helpers for running external processes in end-to-end tests.
 */

#pragma once

#include <array>
#include <cstdio>
#include <string>

#if defined(_WIN32)
#define popen _popen
#define pclose _pclose
#endif

namespace scope::test_support
{

inline std::string quoteArgument(const std::string& value)
{
    return '"' + value + '"';
}

inline std::string captureCommandOutput(const std::string& command)
{
    std::string output;
    std::array<char, 256> buffer{};

    FILE* pipe = popen(command.c_str(), "r");

    if (pipe == nullptr)
    {
        return {};
    }

    while (std::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
    {
        output += buffer.data();
    }

    pclose(pipe);

    return output;
}

} // namespace scope::test_support
