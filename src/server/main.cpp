#include "server.hpp"
#include <iostream>
#include <csignal>

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
    (void)argc;
    (void)argv;

    std::cout << "City Server v0.1.0\n";

    // Set up signal handling
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    city::Server server;
    g_server = &server;

    if (!server.init()) {
        std::cerr << "Failed to initialize server\n";
        return 1;
    }

    // TODO: Parse command line arguments for port, etc.
    if (!server.start(7777)) {
        std::cerr << "Failed to start server\n";
        return 1;
    }

    server.run();

    g_server = nullptr;
    return 0;
}
