#include "client.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    std::cout << "City Client v0.1.0\n";

    city::Client client;

    if (!client.init()) {
        std::cerr << "Failed to initialize client\n";
        return 1;
    }

    client.run();

    return 0;
}
