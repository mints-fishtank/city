#include "client.hpp"
#include "render/renderer.hpp"
#include "input/input_manager.hpp"
#include "prediction/prediction.hpp"
#include "net/client_connection.hpp"
#include "net/content_downloader.hpp"

#include <SDL3/SDL.h>
#include <iostream>
#include <chrono>
#include <thread>

namespace city {

Client::Client() = default;
Client::~Client() {
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
    prediction_ = std::make_unique<PredictionSystem>(world_);
    connection_ = std::make_unique<ClientConnection>();
    content_ = std::make_unique<ContentDownloader>();

    std::cout << "Client initialized successfully\n";
    return true;
}

void Client::run() {
    running_ = true;

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

    // Update input
    input_->update();

    // Update prediction
    prediction_->update(dt);

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
            case SDL_EVENT_KEY_UP:
                if (input_) {
                    input_->handle_event(event);
                }
                break;

            default:
                break;
        }
    }
}

void Client::process_network() {
    if (!connection_) return;

    connection_->update();

    // Process received messages
    while (auto msg = connection_->receive()) {
        // TODO: Handle messages
        (void)msg;
    }
}

} // namespace city
