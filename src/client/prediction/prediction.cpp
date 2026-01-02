#include "prediction.hpp"
#include "core/game/components/transform.hpp"
#include "core/game/components/player.hpp"
#include "core/game/systems/movement.hpp"

namespace city {

PredictionSystem::PredictionSystem(World& world, TileMap& tilemap)
    : world_(world), tilemap_(tilemap) {}

void PredictionSystem::update(f32 dt) {
    Entity player_entity = world_.get_by_net_id(local_player_id_);
    if (!player_entity.is_valid()) return;

    auto* transform = world_.get_component<Transform>(player_entity);
    auto* player = world_.get_component<Player>(player_entity);
    if (!transform || !player) return;

    // Use the SAME movement logic as the server
    MoverSystem::update_movement(*transform, *player, tilemap_, dt);
}

void PredictionSystem::record_input(InputSnapshot input, [[maybe_unused]] f32 dt) {
    input_buffer_.add(input);

    Entity player_entity = world_.get_by_net_id(local_player_id_);
    if (!player_entity.is_valid()) return;

    auto* player = world_.get_component<Player>(player_entity);
    if (!player) return;

    // Use the SAME input application as the server
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

Vec2f PredictionSystem::get_predicted_position(NetEntityId net_id) const {
    Entity e = world_.get_by_net_id(net_id);
    if (!e.is_valid()) return {0.0f, 0.0f};

    const auto* transform = world_.get_component<Transform>(e);
    return transform ? transform->position : Vec2f{0.0f, 0.0f};
}

void PredictionSystem::reconcile([[maybe_unused]] u32 server_tick, const EntityState& server_state) {
    Entity player_entity = world_.get_by_net_id(local_player_id_);
    if (!player_entity.is_valid()) return;

    auto* transform = world_.get_component<Transform>(player_entity);
    auto* player = world_.get_component<Player>(player_entity);
    if (!transform || !player) return;

    // Position-based reconciliation - compare actual positions
    f32 dx = transform->position.x - server_state.position.x;
    f32 dy = transform->position.y - server_state.position.y;
    f32 distance_sq = dx * dx + dy * dy;

    // Tolerance: allow up to ~1.5 tiles of prediction error before correcting
    // This accounts for network latency and prediction lookahead
    constexpr f32 POSITION_TOLERANCE = 1.5f;
    constexpr f32 TOLERANCE_SQ = POSITION_TOLERANCE * POSITION_TOLERANCE;

    // Within tolerance - we're close enough, no correction needed
    if (distance_sq <= TOLERANCE_SQ) {
        return;
    }

    // Significant position desync - must correct
    // Snap to server position and state
    transform->position = server_state.position;
    transform->velocity = server_state.velocity;

    // For grid-locked mode, also sync grid state
    if (player->movement_mode == MovementMode::GridLocked) {
        player->grid_pos = server_state.grid_pos;
        player->move_target = server_state.move_target;
        player->is_moving = server_state.is_moving;
        player->queued_direction = {0, 0};
    }
}

} // namespace city
