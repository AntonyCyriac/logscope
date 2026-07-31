/**
 * @file main.cpp
 * @brief logscope-web entry point (M15.1).
 */

#include "web_config.hpp"
#include "web_server.hpp"
#include "web_startup.hpp"

#include <iostream>

#ifndef LOGSCOPE_VERSION
#define LOGSCOPE_VERSION "unknown"
#endif

int main(int argc, char* argv[])
{
    const auto configResult = scope::web::loadStartupConfig(argc, argv);

    if (!configResult)
    {
        if (configResult.error().message() == "Help requested.")
        {
            return 0;
        }

        std::cerr << configResult.error().message() << std::endl;

        return 1;
    }

    const scope::web::WebConfig& config = *configResult;
    scope::web::WebServer server(config);

    std::cout << "logscope-web " << LOGSCOPE_VERSION << " listening on " << config.listenUrl() << std::endl;

    if (!server.run())
    {
        std::cerr << "Failed to start logscope-web." << std::endl;

        return 1;
    }

    return 0;
}
