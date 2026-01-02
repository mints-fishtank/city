#include "server.hpp"
#include "net/server_connection.hpp"
#include "net/client_session.hpp"
#include "simulation/game_state.hpp"
#include "simulation/round_manager.hpp"
#include "systems/input_processor.hpp"
#include "systems/entity_sync.hpp"
#include "core/game/components/transform.hpp"
#include "core/game/components/player.hpp"

#include <iostream>
#include <chrono>
#include <thread>

namespace city {

Server::Server() = default;
Server::~Server() = default;

bool Server::init() {
    // Initialize subsystems
    connection_ = std::make_unique<ServerConnection>(*this);
    game_state_ = std::make_unique<GameState>(world_, tilemap_);
    round_manager_ = std::make_unique<RoundManager>(*this);
    input_processor_ = std::make_unique<InputProcessor>(world_, tilemap_);
    entity_sync_ = std::make_unique<EntitySync>(world_);

    // Load content manifest
    manifest_ = ContentManifest::from_directory("content", "official");
    manifest_.server_name = "City Server";

    // Create a simple test map
    tilemap_.set_bounds(64, 64);
    Tile floor_tile;
    floor_tile.floor_id = 1;
    floor_tile.flags = TileFlags::None;

    Tile wall_tile;
    wall_tile.floor_id = 1;
    wall_tile.wall_id = 1;
    wall_tile.flags = TileFlags::Solid | TileFlags::Opaque;

    // Fill map with floor
    for (i32 y = 0; y < 64; ++y) {
        for (i32 x = 0; x < 64; ++x) {
            // Border walls
            if (x == 0 || x == 63 || y == 0 || y == 63) {
                tilemap_.set_tile({x, y}, wall_tile);
            } else {
                tilemap_.set_tile({x, y}, floor_tile);
            }
        }
    }

    std::cout << "Server initialized\n";
    return true;
}

bool Server::start(u16 port) {
    if (!connection_->start(port)) {
        return false;
    }

    running_ = true;
    std::cout << "Server started on port " << port << "\n";
    return true;
}

void Server::stop() {
    running_ = false;
}

void Server::run() {
    auto last_time = std::chrono::high_resolution_clock::now();
    constexpr f32 fixed_dt = net::TICK_INTERVAL;
    f32 accumulator = 0.0f;

    while (running_) {
        auto current_time = std::chrono::high_resolution_clock::now();
        f32 dt = std::chrono::duration<f32>(current_time - last_time).count();
        last_time = current_time;

        // Cap delta time
        if (dt > 0.25f) dt = 0.25f;

        accumulator += dt;

        // Process network at frame rate
        process_network();

        // Fixed timestep for simulation
        while (accumulator >= fixed_dt) {
            update(fixed_dt);
            ++current_tick_;
            accumulator -= fixed_dt;

            // Broadcast state after each tick
            broadcast_state();
        }

        // Sleep to avoid spinning
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "Server stopped\n";
}

void Server::update(f32 dt) {
    // Process queued inputs
    input_processor_->update(world_, dt);

    // Update game systems
    world_.update(dt);

    // Update round state
    round_manager_->update(dt);
}

void Server::process_network() {
    connection_->update();
}

void Server::broadcast_state() {
    // Build delta state and send to all clients
    entity_sync_->broadcast(*connection_, current_tick_);
}

void Server::on_client_connected(ClientSession& session) {
    std::cout << "Client connected: " << session.name() << "\n";

    // Create player entity
    Entity player = world_.create();
    NetEntityId net_id = world_.allocate_net_id();
    world_.assign_net_id(player, net_id);

    Vec2i spawn_tile{32, 32};
    Vec2f spawn_pos{
        static_cast<f32>(spawn_tile.x) + 0.5f,
        static_cast<f32>(spawn_tile.y) + 0.5f
    };

    world_.add_component<Transform>(player, Transform{
        .position = spawn_pos,
        .velocity = {0.0f, 0.0f},
        .rotation = 0.0f
    });

    world_.add_component<Player>(player, Player{
        .name = session.name(),
        .session_id = session.id(),
        .team = 0,
        .is_local = false,
        .grid_pos = spawn_tile,
        .move_target = spawn_tile,
        .move_progress = 0.0f,
        .is_moving = false
    });

    session.set_player_entity(net_id);

    // Send server hello
    net::ServerHelloPayload hello{
        .protocol_version = net::PROTOCOL_VERSION,
        .server_id = manifest_.server_id,
        .server_name = manifest_.server_name,
        .session_id = session.id(),
        .player_entity_id = net_id
    };

    session.send(net::Message::create(net::MessageType::ServerHello, hello));

    // Send existing players to the new client
    world_.each<Transform, Player>([&](Entity e, Transform& t, Player& p) {
        NetEntityId existing_id = world_.get_net_id(e);
        if (existing_id == net_id) return;  // Skip self

        net::EntitySpawnPayload spawn{
            .entity_id = existing_id,
            .position = t.position,
            .name = p.name,
            .is_player = true
        };
        session.send(net::Message::create(net::MessageType::EntitySpawn, spawn));
    });

    // Broadcast new player to all existing clients
    net::EntitySpawnPayload spawn{
        .entity_id = net_id,
        .position = spawn_pos,
        .name = session.name(),
        .is_player = true
    };
    connection_->broadcast(net::Message::create(net::MessageType::EntitySpawn, spawn));
}

void Server::on_client_disconnected(ClientSession& session) {
    std::cout << "Client disconnected: " << session.name() << "\n";

    // Broadcast despawn to all clients
    net::EntityDespawnPayload despawn{
        .entity_id = session.player_entity()
    };
    connection_->broadcast(net::Message::create(net::MessageType::EntityDespawn, despawn));

    // Remove player entity
    Entity player = world_.get_by_net_id(session.player_entity());
    if (player.is_valid()) {
        world_.destroy(player);
    }
}

void Server::on_client_message(ClientSession& session, const net::Message& msg) {
    switch (msg.type()) {
        case net::MessageType::PlayerInput: {
            net::PlayerInputPayload input;
            auto reader = msg.reader();
            input.deserialize(reader);
            input_processor_->set_input(session.player_entity(), input);
            break;
        }

        case net::MessageType::ChatMessage: {
            net::ChatPayload chat;
            auto reader = msg.reader();
            chat.deserialize(reader);
            // Broadcast to all clients
            connection_->broadcast(net::Message::create(net::MessageType::ChatBroadcast, chat));
            break;
        }

        default:
            break;
    }
}

} // namespace city
