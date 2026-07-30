/**
 * @file query_filter_fuzz.cpp
 * @brief libFuzzer smoke target for filter DSL and search query parsers (Phase 1).
 */

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "query_parser.hpp"
#include "search_query.hpp"
#include "search_query_parser.hpp"

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, const std::size_t size)
{
    if (size <= 1U)
    {
        return 0;
    }

    const std::string_view input(reinterpret_cast<const char*>(data + 1), size - 1U);

    switch (data[0] % 3U)
    {
    case 0U:
        (void)scope::query::parseFilterQuery(input);
        break;
    case 1U:
        (void)scope::search::parseSearchQuery(input);
        break;
    default:
        const auto caseSensitivity =
            (data[0] % 2U == 0U) ? scope::search::CaseSensitivity::Sensitive
                                 : scope::search::CaseSensitivity::Insensitive;
        (void)scope::search::parseRegexQuery(input, caseSensitivity);
        break;
    }

    return 0;
}
