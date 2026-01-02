#include "client.hpp"
#include "core/net/protocol.hpp"
#include <iostream>
#include <string>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <timeapi.h>
#endif

void print_usage(const char* program) {
    std::cout << "Usage: " << program << " [options]\n"
              << "Options:\n"
              << "  --connect <host:port>  Connect to a server (default: standalone mode)\n"
              << "  --name <name>          Set player name\n"
              << "  --help                 Show this help\n";
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    // Set Windows timer resolution to 1ms for accurate sleep timing
    // Without this, sleep_for(1ms) can sleep up to 15.6ms
    timeBeginPeriod(1);
#endif

    std::cout << "City Client v0.1.0\n";

    std::string connect_host;
    city::u16 connect_port = city::net::DEFAULT_PORT;
    std::string player_name = "Player";

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--connect" && i + 1 < argc) {
            std::string addr = argv[++i];
            auto colon = addr.find(':');
            if (colon != std::string::npos) {
                connect_host = addr.substr(0, colon);
                connect_port = static_cast<city::u16>(std::stoi(addr.substr(colon + 1)));
            } else {
                connect_host = addr;
            }
        } else if (arg == "--name" && i + 1 < argc) {
            player_name = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        }
    }

    city::Client client;
    client.set_player_name(player_name);

    if (!client.init()) {
        std::cerr << "Failed to initialize client\n";
#ifdef _WIN32
        timeEndPeriod(1);
#endif
        return 1;
    }

    // Connect to server if specified
    if (!connect_host.empty()) {
        std::cout << "Connecting to " << connect_host << ":" << connect_port << "...\n";
        if (!client.connect(connect_host, connect_port)) {
            std::cerr << "Failed to connect to server\n";
#ifdef _WIN32
            timeEndPeriod(1);
#endif
            return 1;
        }
    } else {
        std::cout << "Running in standalone mode (use --connect to join a server)\n";
    }

    client.run();

#ifdef _WIN32
    timeEndPeriod(1);
#endif
    return 0;
}
