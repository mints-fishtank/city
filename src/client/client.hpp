#pragma once

#include "core/ecs/world.hpp"
#include "core/grid/tilemap.hpp"
#include "core/net/protocol.hpp"
#include <memory>
#include <string>

namespace city {

// Forward declarations
class Renderer;
class InputManager;
class PredictionSystem;
class ClientConnection;
class ContentDownloader;

// Client state
enum class ClientState {
    Disconnected,
    Connecting,
    DownloadingContent,
    Playing,
};

// Main client class
class Client {
public:
    Client();
    ~Client();

    // Initialize SDL, Vulkan, etc.
    bool init();

    // Main game loop
    void run();

    // Connect to a server
    bool connect(const std::string& host, u16 port = net::DEFAULT_PORT);

    // Disconnect from server
    void disconnect();

    // Get current state
    ClientState state() const { return state_; }

    // Get the local player entity
    Entity local_player() const { return local_player_; }

private:
    void update(f32 dt);
    void render();
    void handle_events();
    void process_network();

    ClientState state_{ClientState::Disconnected};
    bool running_{false};

    // Game world
    World world_;
    TileMap tilemap_;
    Entity local_player_{Entity::null()};

    // Subsystems (will be implemented later)
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<InputManager> input_;
    std::unique_ptr<PredictionSystem> prediction_;
    std::unique_ptr<ClientConnection> connection_;
    std::unique_ptr<ContentDownloader> content_;

    // Timing
    u32 current_tick_{0};
    f32 tick_accumulator_{0.0f};
};

} // namespace city
