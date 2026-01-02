#include "client.hpp"
#include "render/renderer.hpp"
#include "input/input_manager.hpp"
#include "prediction/prediction.hpp"
#include "prediction/interpolation.hpp"
#include "net/client_connection.hpp"
#include "net/content_downloader.hpp"
#include "core/game/components/transform.hpp"
#include "core/game/components/player.hpp"
#include "server/server.hpp"

#include <SDL3/SDL.h>
#include <iostream>
#include <chrono>
#include <thread>

namespace city {

Client::Client() = default;

Client::~Client() {
    stop_local_server();
    SDL_Quit();
}

bool Client::init() {
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    // Create subsystems
    renderer_ = std::make_unique<Renderer>();
    if (!renderer_->init(1280, 720, "City")) {
        std::cerr << "Renderer init failed\n";
        return false;
    }

    input_ = std::make_unique<InputManager>();
    prediction_ = std::make_unique<PredictionSystem>(world_, tilemap_);
    interpolation_ = std::make_unique<InterpolationSystem>();
    connection_ = std::make_unique<ClientConnection>();
    content_ = std::make_unique<ContentDownloader>();

    std::cout << "Client initialized successfully\n";
    std::cout << "Use WASD or Arrow keys to move, +/- to zoom\n";
    return true;
}

void Client::start_local_server() {
    std::cout << "Starting local server...\n";

    local_server_ = std::make_unique<Server>();
    if (!local_server_->init()) {
        std::cerr << "Failed to initialize local server\n";
        return;
    }

    if (!local_server_->start(net::DEFAULT_PORT)) {
        std::cerr << "Failed to start local server\n";
        return;
    }

    // Run server on separate thread
    server_thread_ = std::make_unique<std::thread>([this]() {
        local_server_->run();
    });

    using_local_server_ = true;

    // Give the server a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Connect to local server
    if (!connect("localhost", net::DEFAULT_PORT)) {
        std::cerr << "Failed to connect to local server\n";
        stop_local_server();
    }
}

void Client::stop_local_server() {
    if (local_server_) {
        local_server_->stop();
    }

    if (server_thread_ && server_thread_->joinable()) {
        server_thread_->join();
    }

    server_thread_.reset();
    local_server_.reset();
    using_local_server_ = false;
}

void Client::run() {
    running_ = true;

    // If not already connecting to a server, start a local one
    if (state_ == ClientState::Disconnected) {
        start_local_server();
    }

    auto last_time = std::chrono::high_resolution_clock::now();
    constexpr f32 fixed_dt = net::TICK_INTERVAL;

    while (running_) {
        auto current_time = std::chrono::high_resolution_clock::now();
        f32 dt = std::chrono::duration<f32>(current_time - last_time).count();
        last_time = current_time;

        // Cap delta time to avoid spiral of death
        if (dt > 0.25f) dt = 0.25f;

        handle_events();

        if (!running_) break;

        // Fixed timestep for game logic
        tick_accumulator_ += dt;
        while (tick_accumulator_ >= fixed_dt) {
            update(fixed_dt);
            tick_accumulator_ -= fixed_dt;
            ++current_tick_;
        }

        process_network();
        render();

        // Small sleep to avoid spinning
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Clean up local server if we started one
    if (using_local_server_) {
        stop_local_server();
    }
}

bool Client::connect(const std::string& host, u16 port) {
    if (!connection_->connect(host, port)) {
        return false;
    }
    state_ = ClientState::Connecting;
    return true;
}

void Client::disconnect() {
    if (connection_) {
        connection_->disconnect();
    }
    state_ = ClientState::Disconnected;
    local_player_ = Entity::null();
}

void Client::update(f32 dt) {
    if (state_ != ClientState::Playing) return;

    // Update interpolation for remote entities
    interpolation_->update(dt);

    // Apply interpolated positions to remote entities
    world_.each<Transform, Player>([this](Entity e, Transform& transform, Player& player) {
        if (player.is_local) return;  // Skip local player

        NetEntityId net_id = world_.get_net_id(e);
        if (net_id != INVALID_NET_ENTITY_ID && interpolation_->is_tracking(net_id)) {
            transform.position = interpolation_->get_position(net_id, transform.position);
        }
    });

    // Get local player components
    auto* transform = world_.get_component<Transform>(local_player_);
    auto* player = world_.get_component<Player>(local_player_);

    if (transform && player) {
        // Record input with prediction system
        InputSnapshot input{
            .tick = current_tick_,
            .move_x = static_cast<i8>(player->input_direction.x),
            .move_y = static_cast<i8>(player->input_direction.y),
            .interact = false,
            .secondary = false,
            .target_tile = {0, 0}
        };
        prediction_->record_input(input, dt);

        // Update grid movement (handles animation and collision)
        prediction_->update(dt);

        // Update camera to follow player
        renderer_->set_camera_position(transform->position);
    }

    // Update world systems
    world_.update(dt);
}

void Client::render() {
    renderer_->begin_frame();

    if (state_ == ClientState::Playing) {
        renderer_->render_tilemap(tilemap_);
        renderer_->render_entities(world_);
    }

    renderer_->end_frame();
}

void Client::handle_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                running_ = false;
                break;

            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP: {
                bool pressed = (event.type == SDL_EVENT_KEY_DOWN);

                // Handle movement directly for now
                auto* player = world_.get_component<Player>(local_player_);
                if (player) {
                    switch (event.key.scancode) {
                        case SDL_SCANCODE_W:
                        case SDL_SCANCODE_UP:
                            player->input_direction.y = pressed ? -1 : 0;
                            break;
                        case SDL_SCANCODE_S:
                        case SDL_SCANCODE_DOWN:
                            player->input_direction.y = pressed ? 1 : 0;
                            break;
                        case SDL_SCANCODE_A:
                        case SDL_SCANCODE_LEFT:
                            player->input_direction.x = pressed ? -1 : 0;
                            break;
                        case SDL_SCANCODE_D:
                        case SDL_SCANCODE_RIGHT:
                            player->input_direction.x = pressed ? 1 : 0;
                            break;
                        case SDL_SCANCODE_EQUALS:
                        case SDL_SCANCODE_KP_PLUS:
                            if (pressed) renderer_->set_camera_zoom(renderer_->camera_zoom() * 1.2f);
                            break;
                        case SDL_SCANCODE_MINUS:
                        case SDL_SCANCODE_KP_MINUS:
                            if (pressed) renderer_->set_camera_zoom(renderer_->camera_zoom() / 1.2f);
                            break;
                        case SDL_SCANCODE_ESCAPE:
                            if (pressed) running_ = false;
                            break;
                        default:
                            break;
                    }
                }
                break;
            }

            default:
                break;
        }
    }
}

void Client::process_network() {
    if (!connection_) return;

    connection_->update();

    // Check connection state transitions
    if (state_ == ClientState::Connecting &&
        connection_->state() == ConnectionState::Connected) {
        // Send client hello
        net::ClientHelloPayload hello{
            .protocol_version = net::PROTOCOL_VERSION,
            .client_version = "0.1.0",
            .player_name = player_name_
        };
        connection_->send(net::Message::create(net::MessageType::ClientHello, hello));
        std::cout << "Sent ClientHello\n";
        // Wait for ServerHello - use DownloadingContent as intermediate state
        state_ = ClientState::DownloadingContent;
    }

    // Process received messages
    while (auto msg = connection_->receive()) {
        switch (msg->type()) {
            case net::MessageType::ServerHello:
                handle_server_hello(*msg);
                break;
            case net::MessageType::EntitySpawn:
                handle_entity_spawn(*msg);
                break;
            case net::MessageType::EntityDespawn:
                handle_entity_despawn(*msg);
                break;
            case net::MessageType::DeltaState:
                handle_delta_state(*msg);
                break;
            default:
                break;
        }
    }

    // Send input if playing
    if (state_ == ClientState::Playing &&
        connection_->state() == ConnectionState::Connected) {
        send_input();
    }
}

void Client::handle_server_hello(const net::Message& msg) {
    net::ServerHelloPayload hello;
    auto reader = msg.reader();
    hello.deserialize(reader);

    session_id_ = hello.session_id;
    player_net_id_ = hello.player_entity_id;

    std::cout << "Connected to " << hello.server_name
              << " (session " << session_id_ << ")\n";
    std::cout << "Player entity: " << player_net_id_ << "\n";

    // Create temporary tilemap until server sends real map data
    tilemap_.set_bounds(64, 64);
    Tile floor_tile;
    floor_tile.floor_id = 1;
    floor_tile.flags = TileFlags::None;
    Tile wall_tile;
    wall_tile.floor_id = 1;
    wall_tile.wall_id = 1;
    wall_tile.flags = TileFlags::Solid | TileFlags::Opaque;

    for (i32 y = 0; y < 64; ++y) {
        for (i32 x = 0; x < 64; ++x) {
            if (x == 0 || x == 63 || y == 0 || y == 63) {
                tilemap_.set_tile({x, y}, wall_tile);
            } else {
                tilemap_.set_tile({x, y}, floor_tile);
            }
        }
    }

    // Create local player entity with the server-assigned net ID
    Vec2i spawn_tile{32, 32};
    Vec2f spawn_pos{
        static_cast<f32>(spawn_tile.x) + 0.5f,
        static_cast<f32>(spawn_tile.y) + 0.5f
    };

    local_player_ = world_.create();
    world_.assign_net_id(local_player_, player_net_id_);

    world_.add_component<Transform>(local_player_, Transform{
        .position = spawn_pos,
        .velocity = {0.0f, 0.0f},
        .rotation = 0.0f
    });
    world_.add_component<Player>(local_player_, Player{
        .name = player_name_,
        .session_id = session_id_,
        .team = 0,
        .is_local = true,
        .grid_pos = spawn_tile,
        .move_target = spawn_tile,
        .is_moving = false
    });

    renderer_->set_camera_position(spawn_pos);
    prediction_->set_local_player(player_net_id_);
    state_ = ClientState::Playing;
}

void Client::handle_entity_spawn(const net::Message& msg) {
    net::EntitySpawnPayload spawn;
    auto reader = msg.reader();
    spawn.deserialize(reader);

    // Skip if it's our own entity
    if (spawn.entity_id == player_net_id_) return;

    // Check if entity already exists
    Entity existing = world_.get_by_net_id(spawn.entity_id);
    if (existing.is_valid()) return;

    // Create remote player entity
    Entity remote = world_.create();
    world_.assign_net_id(remote, spawn.entity_id);

    // Calculate grid position from spawn position
    Vec2i grid_pos{
        static_cast<i32>(spawn.position.x),
        static_cast<i32>(spawn.position.y)
    };

    world_.add_component<Transform>(remote, Transform{
        .position = spawn.position,
        .velocity = {0.0f, 0.0f},
        .rotation = 0.0f
    });

    if (spawn.is_player) {
        world_.add_component<Player>(remote, Player{
            .name = spawn.name,
            .session_id = 0,
            .team = 0,
            .is_local = false,
            .grid_pos = grid_pos,
            .move_target = grid_pos,
            .is_moving = false
        });
        std::cout << "Player joined: " << spawn.name << "\n";
    }
}

void Client::handle_entity_despawn(const net::Message& msg) {
    net::EntityDespawnPayload despawn;
    auto reader = msg.reader();
    despawn.deserialize(reader);

    // Skip if it's our own entity
    if (despawn.entity_id == player_net_id_) return;

    // Remove from interpolation tracking
    interpolation_->remove(despawn.entity_id);

    Entity entity = world_.get_by_net_id(despawn.entity_id);
    if (entity.is_valid()) {
        auto* player = world_.get_component<Player>(entity);
        if (player) {
            std::cout << "Player left: " << player->name << "\n";
        }
        world_.destroy(entity);
    }
}

void Client::handle_delta_state(const net::Message& msg) {
    auto reader = msg.reader();

    u32 tick = reader.read_u32();
    last_server_tick_ = tick;

    u32 count = reader.read_u32();

    // Collect states for reconciliation
    std::vector<EntityState> server_states;
    server_states.reserve(count);

    for (u32 i = 0; i < count; ++i) {
        NetEntityId net_id = reader.read_u32();
        Vec2f position = reader.read_vec2f();
        Vec2f velocity = reader.read_vec2f();
        bool has_player = reader.read_bool();
        bool is_moving = false;
        Vec2i grid_pos{0, 0};
        Vec2i move_target{0, 0};
        if (has_player) {
            is_moving = reader.read_bool();
            grid_pos = reader.read_vec2i();
            move_target = reader.read_vec2i();
        }

        // Collect state for local player reconciliation
        if (net_id == player_net_id_) {
            server_states.push_back(EntityState{
                .net_id = net_id,
                .position = position,
                .velocity = velocity,
                .grid_pos = grid_pos,
                .move_target = move_target,
                .is_moving = is_moving
            });
            continue;
        }

        // Update remote entity with interpolation
        Entity entity = world_.get_by_net_id(net_id);
        if (entity.is_valid()) {
            // Set target position for interpolation
            interpolation_->set_target(net_id, position);

            auto* transform = world_.get_component<Transform>(entity);
            if (transform) {
                transform->velocity = velocity;
            }
            if (has_player) {
                auto* player = world_.get_component<Player>(entity);
                if (player) {
                    player->is_moving = is_moving;
                }
            }
        }
    }

    // Reconcile local player prediction
    if (!server_states.empty()) {
        prediction_->on_server_state(tick, server_states);
    }
}

void Client::handle_entity_update(const net::Message& msg) {
    // Not used - we use DeltaState instead
    (void)msg;
}

void Client::send_input() {
    auto* player = world_.get_component<Player>(local_player_);
    if (!player) return;

    net::PlayerInputPayload input{
        .tick = current_tick_,
        .last_received_tick = last_server_tick_,
        .move_x = static_cast<i8>(player->input_direction.x),
        .move_y = static_cast<i8>(player->input_direction.y),
        .buttons = 0,
        .target_tile = {0, 0}
    };

    connection_->send(
        net::Message::create(net::MessageType::PlayerInput, input),
        net::Reliability::UnreliableSequenced
    );
}

} // namespace city
