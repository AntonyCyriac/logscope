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

LogScopeFormatParser* createFormatParser()
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

} // namespace

extern "C" int logscope_plugin_register(const LogScopeHostApi* host)
{
    if (host == nullptr)
    {
        return 1;
    }

    const LogScopePluginInfo info{LOGSCOPE_PLUGIN_API_VERSION, "sample.parser", "1.0.0",
                                  "Sample pipe-delimited format parser plugin."};

    if (host->register_extension(host->context, &info) != 0)
    {
        return 1;
    }

    return host->register_format_parser(host->context, "pipe-delimited", &createFormatParser);
}
