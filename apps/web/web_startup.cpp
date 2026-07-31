/**
 * @file web_startup.cpp
 */

#include "web_startup.hpp"

#include "configuration_manager.hpp"

#include <iostream>
#include <optional>
#include <string>

namespace scope::web
{

namespace
{

struct WebCliOptions
{
    bool help = false;
    bool configPathSet = false;
    foundation::Path configPath;
    std::optional<std::string> bindHost;
    std::optional<int> bindPort;
    std::optional<foundation::Path> tlsCertPath;
    std::optional<foundation::Path> tlsKeyPath;
};

bool isFlag(const std::string& argument)
{
    return argument.size() >= 2U && argument[0] == '-' && argument[1] == '-';
}

foundation::Result<std::string> requireValue(const int argc, char* argv[], const int index, const char* flagName)
{
    if (index + 1 >= argc)
    {
        return foundation::Result<std::string>(
            foundation::Error(foundation::ErrorCode::InvalidArgument,
                              std::string("Missing value for ") + flagName + "."));
    }

    return foundation::Result<std::string>(std::string(argv[index + 1]));
}

foundation::Result<WebCliOptions> parseCliOptions(const int argc, char* argv[])
{
    WebCliOptions options;

    for (int index = 1; index < argc; ++index)
    {
        const std::string argument(argv[index]);

        if (argument == "--help" || argument == "-h")
        {
            options.help = true;

            continue;
        }

        if (argument == "--config")
        {
            const auto value = requireValue(argc, argv, index, "--config");

            if (!value)
            {
                return foundation::Result<WebCliOptions>(value.error());
            }

            options.configPathSet = true;
            options.configPath = foundation::Path(*value);
            ++index;

            continue;
        }

        if (argument == "--bind-host")
        {
            const auto value = requireValue(argc, argv, index, "--bind-host");

            if (!value)
            {
                return foundation::Result<WebCliOptions>(value.error());
            }

            options.bindHost = *value;
            ++index;

            continue;
        }

        if (argument == "--bind-port")
        {
            const auto value = requireValue(argc, argv, index, "--bind-port");

            if (!value)
            {
                return foundation::Result<WebCliOptions>(value.error());
            }

            options.bindPort = std::stoi(*value);
            ++index;

            continue;
        }

        if (argument == "--tls-cert")
        {
            const auto value = requireValue(argc, argv, index, "--tls-cert");

            if (!value)
            {
                return foundation::Result<WebCliOptions>(value.error());
            }

            options.tlsCertPath = foundation::Path(*value);
            ++index;

            continue;
        }

        if (argument == "--tls-key")
        {
            const auto value = requireValue(argc, argv, index, "--tls-key");

            if (!value)
            {
                return foundation::Result<WebCliOptions>(value.error());
            }

            options.tlsKeyPath = foundation::Path(*value);
            ++index;

            continue;
        }

        if (isFlag(argument))
        {
            return foundation::Result<WebCliOptions>(
                foundation::Error(foundation::ErrorCode::InvalidArgument, "Unknown option: " + argument));
        }

        return foundation::Result<WebCliOptions>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Unexpected argument: " + argument));
    }

    return foundation::Result<WebCliOptions>(std::move(options));
}

} // namespace

void printWebUsage(std::ostream& output)
{
    output << "Usage: logscope-web [options]\n\n"
           << "Options:\n"
           << "  --config <file>       Load web.* settings from a properties file\n"
           << "  --bind-host <host>    Listen address (default 127.0.0.1)\n"
           << "  --bind-port <port>    Listen port (default 8080)\n"
           << "  --tls-cert <file>     TLS certificate (PEM); requires OpenSSL build\n"
           << "  --tls-key <file>      TLS private key (PEM); requires OpenSSL build\n"
           << "  --help                Show this help\n\n"
           << "Environment: LOGSCOPE_WEB_BIND_HOST, LOGSCOPE_WEB_BIND_PORT, LOGSCOPE_WEB_API_KEY,\n"
           << "  LOGSCOPE_WEB_TLS_CERT, LOGSCOPE_WEB_TLS_KEY, LOGSCOPE_WEB_UI_DIR\n";
}

foundation::Result<WebConfig> loadStartupConfig(const int argc, char* argv[])
{
    const auto parsedOptions = parseCliOptions(argc, argv);

    if (!parsedOptions)
    {
        return foundation::Result<WebConfig>(parsedOptions.error());
    }

    const WebCliOptions& options = *parsedOptions;

    if (options.help)
    {
        printWebUsage(std::cout);

        return foundation::Result<WebConfig>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Help requested."));
    }

    WebConfig config = WebConfig::defaults();

    if (options.configPathSet)
    {
        const auto loadResult = configuration::ConfigurationManager::loadFromFile(options.configPath);

        if (!loadResult)
        {
            return foundation::Result<WebConfig>(loadResult.error());
        }

        config.mergeFromConfiguration(*loadResult);
    }

    config.applyEnvironment();

    if (options.bindHost.has_value())
    {
        config.bindHost = *options.bindHost;
    }

    if (options.bindPort.has_value())
    {
        config.bindPort = *options.bindPort;
    }

    if (options.tlsCertPath.has_value())
    {
        config.tlsCertPath = *options.tlsCertPath;
    }

    if (options.tlsKeyPath.has_value())
    {
        config.tlsKeyPath = *options.tlsKeyPath;
    }

    if (!config.tlsCertPath.isEmpty() && config.tlsKeyPath.isEmpty())
    {
        return foundation::Result<WebConfig>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "TLS certificate requires TLS private key."));
    }

    if (!config.tlsKeyPath.isEmpty() && config.tlsCertPath.isEmpty())
    {
        return foundation::Result<WebConfig>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "TLS private key requires TLS certificate."));
    }

    config.applyDerivedDefaults();

    return foundation::Result<WebConfig>(std::move(config));
}

} // namespace scope::web
