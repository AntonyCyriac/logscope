/**
 * @file main.cpp
 * @brief logscope-web entry point (M15.1).
 */

#include "web_config.hpp"
#include "web_server.hpp"

#include <iostream>

#ifndef LOGSCOPE_VERSION
#define LOGSCOPE_VERSION "unknown"
#endif

int main()
{
    scope::web::WebConfig config = scope::web::WebConfig::defaults();
    config.applyEnvironment();

    scope::web::WebServer server(config);

    std::cout << "logscope-web listening on http://" << config.bindHost << ':' << config.bindPort << std::endl;

    if (!server.run())
    {
        std::cerr << "Failed to start logscope-web." << std::endl;

        return 1;
    }

    return 0;
}
