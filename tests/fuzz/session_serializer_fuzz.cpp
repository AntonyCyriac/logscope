/**
 * @file session_serializer_fuzz.cpp
 * @brief libFuzzer target for SessionSerializer::deserialize.
 */

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "session_serializer.hpp"

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, const std::size_t size)
{
    const std::string_view content(reinterpret_cast<const char*>(data), size);

    (void)scope::workspace::SessionSerializer::deserialize(content);

    return 0;
}
