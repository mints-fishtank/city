#include "server.hpp"
#include <iostream>
#include <csignal>
#include <cstdlib>

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
        return 1;
    }

    if (!server.start(port)) {
        std::cerr << "Failed to start server\n";
        return 1;
    }

    server.run();

    g_server = nullptr;
    return 0;
}
