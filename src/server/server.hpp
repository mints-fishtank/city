#pragma once

#include "core/ecs/world.hpp"
#include "core/grid/tilemap.hpp"
#include "core/net/protocol.hpp"
#include "core/net/message.hpp"
#include "core/content/content_manifest.hpp"

#ifdef ENABLE_PROFILING
#include "profiling/profiler.hpp"
#include "profiling/profiler_window.hpp"
#endif

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
    // Set embedded=true when running as a local server inside another process (disables GUI)
    bool start(u16 port, bool embedded = false);

    // Stop the server
    void stop();

    // Main server loop
    void run();

    // Accessors
    World& world() { return world_; }
    TileMap& tilemap() { return tilemap_; }
    u32 current_tick() const { return current_tick_; }

#ifdef ENABLE_PROFILING
    // Profiler access
    TickProfiler& profiler() { return profiler_; }
    const TickProfiler& profiler() const { return profiler_; }

    // Enable/disable profiler window
    void set_profiler_window_enabled(bool enabled);
    bool is_profiler_window_enabled() const { return profiler_window_ != nullptr; }
#endif

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

#ifdef ENABLE_PROFILING
    // Profiling
    TickProfiler profiler_;
    std::unique_ptr<ProfilerWindow> profiler_window_;
#endif
};

} // namespace city
