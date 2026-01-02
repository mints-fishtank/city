#include "server.hpp"
#include <iostream>
#include <csignal>
#include <cstdlib>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <timeapi.h>
#endif

static city::Server* g_server = nullptr;

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nShutting down...\n";
        if (g_server) {
            g_server->stop();
        }
    }
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    // Set Windows timer resolution to 1ms for accurate sleep timing
    // Without this, sleep_for(1ms) can sleep up to 15.6ms
    timeBeginPeriod(1);
#endif

    std::cout << "City Server v0.1.0\n";

    // Parse port from command line
    city::u16 port = 7777;
    if (argc > 1) {
        port = static_cast<city::u16>(std::atoi(argv[1]));
    }

    // Set up signal handling
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    city::Server server;
    g_server = &server;

    if (!server.init()) {
        std::cerr << "Failed to initialize server\n";
#ifdef _WIN32
        timeEndPeriod(1);
#endif
        return 1;
    }

    if (!server.start(port)) {
        std::cerr << "Failed to start server\n";
#ifdef _WIN32
        timeEndPeriod(1);
#endif
        return 1;
    }

    server.run();

    g_server = nullptr;
#ifdef _WIN32
    timeEndPeriod(1);
#endif
    return 0;
}
