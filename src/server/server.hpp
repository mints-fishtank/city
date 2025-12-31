#pragma once

#include "core/ecs/world.hpp"
#include "core/grid/tilemap.hpp"
#include "core/net/protocol.hpp"
#include "core/net/message.hpp"
#include "core/content/content_manifest.hpp"
#include <memory>
#include <string>

namespace city {

// Forward declarations
class ServerConnection;
class ClientSession;
class GameState;
class RoundManager;
class InputProcessor;
class EntitySync;

class Server {
public:
    Server();
    ~Server();

    // Initialize server systems
    bool init();

    // Start listening on port
    bool start(u16 port);

    // Stop the server
    void stop();

    // Main server loop
    void run();

    // Accessors
    World& world() { return world_; }
    TileMap& tilemap() { return tilemap_; }
    u32 current_tick() const { return current_tick_; }

    // Handle client events (called by ServerConnection)
    void on_client_connected(ClientSession& session);
    void on_client_disconnected(ClientSession& session);
    void on_client_message(ClientSession& session, const net::Message& msg);

private:
    void update(f32 dt);
    void process_network();
    void broadcast_state();

    bool running_{false};
    u32 current_tick_{0};

    // Game world
    World world_;
    TileMap tilemap_;
    ContentManifest manifest_;

    // Subsystems
    std::unique_ptr<ServerConnection> connection_;
    std::unique_ptr<GameState> game_state_;
    std::unique_ptr<RoundManager> round_manager_;
    std::unique_ptr<InputProcessor> input_processor_;
    std::unique_ptr<EntitySync> entity_sync_;
};

} // namespace city
