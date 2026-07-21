/**
 * @file search_command.cpp
 * @brief Search subcommand implementation.
 */

#include "search_command.hpp"

#include <iostream>

#include "investigate_command.hpp"
#include "output_format.hpp"

namespace scope::cli
{

void printSearchUsage(std::ostream& output)
{
    output << "Usage: logscope search [--config <file>] [--format text|json] "
           << "[--log-format auto|plain|jsonl] [--profile name] [search options] <log-source>\n"
           << "\n"
           << "Search options:\n"
           << "  --search <query>        Simple text search over indexed line content\n"
           << "  --query <expr>          Boolean search expression (AND, OR, NOT, quotes)\n"
           << "  --regex                 Treat --search value as a regular expression\n"
           << "  --case-sensitive        Disable default case-insensitive matching\n"
           << "  --time-from <timestamp> Earliest timestamp (ISO-like)\n"
           << "  --time-to <timestamp>   Latest timestamp (ISO-like)\n"
           << "  --level <name>          Filter by line level: error, warning, info, other\n"
           << "  --message <text>        Filter by message/content substring\n"
           << "  --json-key <key>        Require a JSON top-level key on matching lines\n"
           << "\n"
           << "Log source may be a file path, a directory of .log files, or \"-\" for stdin.\n";
}

int runSearchCommand(const InvestigateOptions& options,
                     configuration::ConfigurationManager& configurationManager,
                     std::ostream& output,
                     std::ostream& errorOutput)
{
    if (options.showHelp)
    {
        printSearchUsage(output);

        return 0;
    }

    if (options.format != OutputFormat::Text && options.format != OutputFormat::Json)
    {
        errorOutput << "Search supports only text or json output formats." << '\n';

        return 1;
    }

    return runInvestigateCommand(options, configurationManager, output, errorOutput);
}

} // namespace scope::cli
