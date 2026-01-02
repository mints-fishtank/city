#pragma once

#include "core/ecs/world.hpp"
#include "core/grid/tilemap.hpp"
#include "core/net/protocol.hpp"
#include "core/net/message.hpp"
#include <memory>
#include <string>
#include <thread>

namespace city {

// Forward declarations
class Renderer;
class InputManager;
class PredictionSystem;
class InterpolationSystem;
class ClientConnection;
class ContentDownloader;
class Server;

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

    // Set player name (before connecting)
    void set_player_name(const std::string& name) { player_name_ = name; }

    // Get current state
    ClientState state() const { return state_; }

    // Get the local player entity
    Entity local_player() const { return local_player_; }

private:
    void update(f32 dt);
    void render();
    void handle_events();
    void process_network();
    void start_local_server();
    void stop_local_server();

    ClientState state_{ClientState::Disconnected};
    bool running_{false};

    // Local server (when running standalone)
    std::unique_ptr<Server> local_server_;
    std::unique_ptr<std::thread> server_thread_;
    bool using_local_server_{false};

    // Game world
    World world_;
    TileMap tilemap_;
    Entity local_player_{Entity::null()};

    // Subsystems
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<InputManager> input_;
    std::unique_ptr<PredictionSystem> prediction_;
    std::unique_ptr<InterpolationSystem> interpolation_;
    std::unique_ptr<ClientConnection> connection_;
    std::unique_ptr<ContentDownloader> content_;

    // Timing
    u32 current_tick_{0};
    f32 tick_accumulator_{0.0f};

    // Networking state
    u32 session_id_{0};
    NetEntityId player_net_id_{0};
    u32 last_server_tick_{0};
    std::string player_name_{"Player"};

    // Message handlers
    void handle_server_hello(const net::Message& msg);
    void handle_entity_spawn(const net::Message& msg);
    void handle_entity_despawn(const net::Message& msg);
    void handle_entity_update(const net::Message& msg);
    void handle_delta_state(const net::Message& msg);
    void send_input();
};

} // namespace city
