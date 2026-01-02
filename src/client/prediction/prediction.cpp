#include "prediction.hpp"
#include "core/game/components/transform.hpp"
#include "core/game/components/player.hpp"
#include "core/game/systems/movement.hpp"
#include "core/net/protocol.hpp"

#include <algorithm>
#include <cmath>

namespace city {

PredictionSystem::PredictionSystem(World& world, TileMap& tilemap)
    : world_(world), tilemap_(tilemap), tick_dt_(net::TICK_INTERVAL) {}

void PredictionSystem::update(f32 dt) {
    Entity player_entity = world_.get_by_net_id(local_player_id_);
    if (!player_entity.is_valid()) return;

    auto* transform = world_.get_component<Transform>(player_entity);
    auto* player = world_.get_component<Player>(player_entity);
    if (!transform || !player) return;

    // Remove visual offset before simulation (position should be authoritative)
    transform->position.x -= position_error_.x;
    transform->position.y -= position_error_.y;

    // Run movement simulation from true position
    MoverSystem::update_movement(*transform, *player, tilemap_, dt);

    // Decay the visual error
    if (position_error_.x != 0.0f || position_error_.y != 0.0f) {
        f32 blend = std::min(1.0f, ERROR_BLEND_RATE * dt);
        position_error_.x *= (1.0f - blend);
        position_error_.y *= (1.0f - blend);

        // Zero out when negligible
        if (std::abs(position_error_.x) < 0.001f) position_error_.x = 0.0f;
        if (std::abs(position_error_.y) < 0.001f) position_error_.y = 0.0f;
    }

    // Re-apply visual offset for rendering
    transform->position.x += position_error_.x;
    transform->position.y += position_error_.y;
}

void PredictionSystem::record_input(InputSnapshot input, [[maybe_unused]] f32 dt) {
    input_buffer_.add(input);

    Entity player_entity = world_.get_by_net_id(local_player_id_);
    if (!player_entity.is_valid()) return;

    auto* player = world_.get_component<Player>(player_entity);
    if (!player) return;

    // Apply input immediately for responsive feel
    MoverSystem::apply_input(*player, {input.move_x, input.move_y});
}

void PredictionSystem::on_server_state(u32 server_tick, const std::vector<EntityState>& states) {
    for (const auto& state : states) {
        if (state.net_id == local_player_id_) {
            reconcile(server_tick, state);
            break;
        }
    }

    input_buffer_.acknowledge(server_tick);
    last_server_tick_ = server_tick;
}

void PredictionSystem::set_local_player(NetEntityId net_id) {
    local_player_id_ = net_id;
}

void PredictionSystem::clear_inputs() {
    input_buffer_.clear();
    position_error_ = {0.0f, 0.0f};
}

Vec2f PredictionSystem::get_predicted_position(NetEntityId net_id) const {
    Entity e = world_.get_by_net_id(net_id);
    if (!e.is_valid()) return {0.0f, 0.0f};

    const auto* transform = world_.get_component<Transform>(e);
    return transform ? transform->position : Vec2f{0.0f, 0.0f};
}

void PredictionSystem::reconcile(u32 server_tick, const EntityState& server_state) {
    Entity player_entity = world_.get_by_net_id(local_player_id_);
    if (!player_entity.is_valid()) return;

    auto* transform = world_.get_component<Transform>(player_entity);
    auto* player = world_.get_component<Player>(player_entity);
    if (!transform || !player) return;

    // Get the TRUE predicted position (without visual error offset)
    Vec2f predicted_pos = {
        transform->position.x - position_error_.x,
        transform->position.y - position_error_.y
    };

    // Reset to authoritative server state (ALL of it, including input_direction)
    // This ensures resimulation starts from the exact same state as the server
    transform->position = server_state.position;
    transform->velocity = server_state.velocity;
    player->grid_pos = server_state.grid_pos;
    player->move_target = server_state.move_target;
    player->is_moving = server_state.is_moving;
    player->input_direction = server_state.input_direction;
    player->queued_direction = {0, 0};

    // Get inputs that haven't been processed by the server yet
    // Server state at tick T includes effects of inputs up to tick T
    // So we only replay inputs from tick T+1 onwards
    auto inputs = input_buffer_.get_inputs_after(server_tick);

    for (const auto& input : inputs) {
        resimulate_tick(input);
    }

    // Calculate the error between old prediction and new resimulated position
    f32 dx = predicted_pos.x - transform->position.x;
    f32 dy = predicted_pos.y - transform->position.y;
    f32 error_sq = dx * dx + dy * dy;

    // Always update position_error_ to the new error (or zero if tiny/huge)
    constexpr f32 MIN_ERROR_SQ = 0.01f;      // Below 0.1 units, just snap (reduces jitter)
    constexpr f32 MAX_ERROR_SQ = 4.0f;       // Above 2 units, just snap (teleport)

    if (error_sq > MIN_ERROR_SQ && error_sq < MAX_ERROR_SQ) {
        position_error_.x = dx;
        position_error_.y = dy;
    } else {
        // Error is negligible or huge - no smoothing needed
        position_error_ = {0.0f, 0.0f};
    }

    // Add the error back to position so it represents the visual position
    transform->position.x += position_error_.x;
    transform->position.y += position_error_.y;
}

void PredictionSystem::resimulate_tick(const InputSnapshot& input) {
    Entity player_entity = world_.get_by_net_id(local_player_id_);
    if (!player_entity.is_valid()) return;

    auto* transform = world_.get_component<Transform>(player_entity);
    auto* player = world_.get_component<Player>(player_entity);
    if (!transform || !player) return;

    // Apply the input
    MoverSystem::apply_input(*player, {input.move_x, input.move_y});

    // Run one tick of movement simulation
    MoverSystem::update_movement(*transform, *player, tilemap_, tick_dt_);
}

} // namespace city
