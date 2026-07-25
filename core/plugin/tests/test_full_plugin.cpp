/**
 * @file test_full_plugin.cpp
 * @brief Integration test plugin exercising report, parser, and search providers (M12).
 */

#include <logscope/plugin/plugin.h>

#include <cstring>

namespace
{

struct PipeParserState
{
    char level[64];
    char timestamp[64];
    char message[256];
};

void destroyPipeParser(void* instance)
{
    delete static_cast<PipeParserState*>(instance);
}

int parsePipeLine(void* instance, const char* line, size_t length, LogScopeParsedLine* out)
{
    auto* state = static_cast<PipeParserState*>(instance);

    if (out == nullptr || state == nullptr)
    {
        return 1;
    }

    out->level = nullptr;
    out->message = nullptr;
    out->timestamp = nullptr;

    if (length == 0U)
    {
        out->outcome = LOGSCOPE_PARSE_BLANK;

        return 0;
    }

    char buffer[512];

    if (length >= sizeof(buffer))
    {
        out->outcome = LOGSCOPE_PARSE_INVALID;

        return 0;
    }

    std::memcpy(buffer, line, length);
    buffer[length] = '\0';

    char* first = std::strchr(buffer, '|');

    if (first == nullptr)
    {
        out->outcome = LOGSCOPE_PARSE_INVALID;

        return 0;
    }

    *first = '\0';
    std::strncpy(state->level, buffer, sizeof(state->level) - 1U);
    state->level[sizeof(state->level) - 1U] = '\0';

    char* second = std::strchr(first + 1, '|');

    if (second == nullptr)
    {
        std::strncpy(state->message, first + 1, sizeof(state->message) - 1U);
        state->message[sizeof(state->message) - 1U] = '\0';
        state->timestamp[0] = '\0';
    }
    else
    {
        *second = '\0';
        std::strncpy(state->timestamp, first + 1, sizeof(state->timestamp) - 1U);
        state->timestamp[sizeof(state->timestamp) - 1U] = '\0';
        std::strncpy(state->message, second + 1, sizeof(state->message) - 1U);
        state->message[sizeof(state->message) - 1U] = '\0';
    }

    out->outcome = LOGSCOPE_PARSE_VALID;
    out->level = state->level;
    out->timestamp = state->timestamp[0] == '\0' ? nullptr : state->timestamp;
    out->message = state->message;

    return 0;
}

const LogScopeFormatParserVTable kPipeParserVTable{destroyPipeParser, parsePipeLine};

LogScopeFormatParser* createPipeParser()
{
    auto* parser = new LogScopeFormatParser();
    auto* state = new PipeParserState();
    state->level[0] = '\0';
    state->timestamp[0] = '\0';
    state->message[0] = '\0';
    parser->instance = state;
    parser->vtable = &kPipeParserVTable;

    return parser;
}

struct ReportContributorState
{
};

void destroyReportContributor(void* instance)
{
    delete static_cast<ReportContributorState*>(instance);
}

int renderReportContributor(void* instance, uint64_t /*totalLines*/, LogScopeReportFragment* out)
{
    (void)instance;

    if (out == nullptr)
    {
        return 1;
    }

    out->text_body = "Plugin report: sample plugin contributor\n";
    out->html_body = "<p>Plugin report: sample plugin contributor</p>";
    out->json_key = "pluginReport";
    out->json_body = "\"plugin\"";

    return 0;
}

const LogScopeReportContributorVTable kReportContributorVTable{destroyReportContributor,
                                                                 renderReportContributor};

LogScopeReportContributor* createReportContributor()
{
    auto* contributor = new LogScopeReportContributor();
    contributor->instance = new ReportContributorState();
    contributor->vtable = &kReportContributorVTable;

    return contributor;
}

struct SearchProviderState
{
};

void destroySearchProvider(void* instance)
{
    delete static_cast<SearchProviderState*>(instance);
}

int searchWithProvider(void* /*instance*/, const char* /*query*/, LogScopeReportFragment* /*out*/)
{
    return 0;
}

const LogScopeSearchProviderVTable kSearchProviderVTable{destroySearchProvider, searchWithProvider};

LogScopeSearchProvider* createSearchProvider()
{
    auto* provider = new LogScopeSearchProvider();
    provider->instance = new SearchProviderState();
    provider->vtable = &kSearchProviderVTable;

    return provider;
}

} // namespace

extern "C" int logscope_plugin_register(const LogScopeHostApi* host)
{
    if (host == nullptr)
    {
        return 1;
    }

    const LogScopePluginInfo info{LOGSCOPE_PLUGIN_API_VERSION, "test.full", "1.0.0",
                                  "Full provider integration test plugin."};

    if (host->register_extension(host->context, &info) != 0)
    {
        return 1;
    }

    if (host->register_report_contributor(host->context, "test.full.report", &createReportContributor) != 0)
    {
        return 1;
    }

    if (host->register_format_parser(host->context, "pipe-delimited", &createPipeParser) != 0)
    {
        return 1;
    }

    if (host->register_search_provider(host->context, "test.full.search", &createSearchProvider) != 0)
    {
        return 1;
    }

    return 0;
}
