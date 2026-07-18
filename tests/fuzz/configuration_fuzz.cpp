/**
 * @file configuration_fuzz.cpp
 * @brief libFuzzer target for ConfigurationManager::loadFromFile.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>

#include "configuration_manager.hpp"

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, const std::size_t size)
{
    const std::string content(reinterpret_cast<const char*>(data), size);
    const std::string path = "fuzz_configuration.properties";

    {
        std::ofstream stream(path);

        stream << content;
    }

    (void)scope::configuration::ConfigurationManager::loadFromFile(path);
    std::remove(path.c_str());

    return 0;
}
