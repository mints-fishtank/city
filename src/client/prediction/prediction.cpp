#include "prediction.hpp"
#include "core/game/components/transform.hpp"
#include "core/game/components/player.hpp"

namespace city {

PredictionSystem::PredictionSystem(World& world, TileMap& tilemap)
    : world_(world), tilemap_(tilemap) {}

void PredictionSystem::update(f32 dt) {
    // Update grid movement for local player
    Entity player_entity = world_.get_by_net_id(local_player_id_);
    if (!player_entity.is_valid()) return;

    auto* transform = world_.get_component<Transform>(player_entity);
    auto* player = world_.get_component<Player>(player_entity);
    if (!transform || !player) return;

    update_grid_movement(*transform, *player, dt);
}

void PredictionSystem::record_input(InputSnapshot input, [[maybe_unused]] f32 dt) {
    input_buffer_.add(input);

    Entity player_entity = world_.get_by_net_id(local_player_id_);
    if (!player_entity.is_valid()) return;

    auto* player = world_.get_component<Player>(player_entity);
    if (!player) return;

    // Store input direction
    player->input_direction = {input.move_x, input.move_y};

    // If currently moving, queue the direction for when move completes
    if (player->is_moving) {
        // Always update queued direction (including clearing it when key released)
        player->queued_direction = {input.move_x, input.move_y};
    }
}

void PredictionSystem::on_server_state(u32 server_tick, const std::vector<EntityState>& states) {
    // Find local player state
    for (const auto& state : states) {
        if (state.net_id == local_player_id_) {
            reconcile(server_tick, state);
            break;
        }
    }

    // Acknowledge processed inputs
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

void PredictionSystem::reconcile([[maybe_unused]] u32 server_tick, const EntityState& authoritative_state) {
    Entity player_entity = world_.get_by_net_id(local_player_id_);
    if (!player_entity.is_valid()) return;

    auto* transform = world_.get_component<Transform>(player_entity);
    auto* player = world_.get_component<Player>(player_entity);
    if (!transform || !player) return;

    // Use server's actual grid state for accurate comparison
    Vec2i server_grid_pos = authoritative_state.grid_pos;
    Vec2i server_move_target = authoritative_state.move_target;
    bool server_is_moving = authoritative_state.is_moving;

    // Determine the tile the server will end up at
    Vec2i server_destination = server_is_moving ? server_move_target : server_grid_pos;

    // Determine the tile the client will end up at
    Vec2i client_destination = player->is_moving ? player->move_target : player->grid_pos;

    // Only reconcile if we disagree on the destination tile
    // This allows client to be ahead of server (finished move that server is still doing)
    if (client_destination.x != server_destination.x || client_destination.y != server_destination.y) {
        // We're heading to the wrong tile - snap to server state
        player->grid_pos = server_grid_pos;
        player->move_target = server_move_target;
        player->is_moving = server_is_moving;
        player->move_progress = 0.0f;
        transform->position = authoritative_state.position;
        transform->velocity = authoritative_state.velocity;

        // Clear queued direction to prevent unintended movement after reconciliation
        player->queued_direction = {0, 0};
    }
}

void PredictionSystem::update_grid_movement(Transform& transform, Player& player, f32 dt) {
    if (player.is_moving) {
        // Advance move progress
        player.move_progress += dt / Player::MOVE_DURATION;

        if (player.move_progress >= 1.0f) {
            // Move complete - snap to target
            player.move_progress = 0.0f;
            player.grid_pos = player.move_target;
            player.is_moving = false;
            transform.position = Vec2f{
                static_cast<f32>(player.grid_pos.x) + 0.5f,
                static_cast<f32>(player.grid_pos.y) + 0.5f
            };
            transform.velocity = {0.0f, 0.0f};

            // Check for queued movement
            if (player.queued_direction.x != 0 || player.queued_direction.y != 0) {
                player.input_direction = player.queued_direction;
                player.queued_direction = {0, 0};
            }
        } else {
            // Interpolate position
            Vec2f from{
                static_cast<f32>(player.grid_pos.x) + 0.5f,
                static_cast<f32>(player.grid_pos.y) + 0.5f
            };
            Vec2f to{
                static_cast<f32>(player.move_target.x) + 0.5f,
                static_cast<f32>(player.move_target.y) + 0.5f
            };
            f32 t = player.move_progress;
            transform.position = Vec2f{
                from.x + (to.x - from.x) * t,
                from.y + (to.y - from.y) * t
            };
        }
    }

    // Try to start a new move if not currently moving
    if (!player.is_moving && (player.input_direction.x != 0 || player.input_direction.y != 0)) {
        Vec2i target{
            player.grid_pos.x + player.input_direction.x,
            player.grid_pos.y + player.input_direction.y
        };

        // Check if target tile is passable
        TilePos tile_pos{target.x, target.y};
        const Tile* tile = tilemap_.get_tile(tile_pos);

        if (tile && tile->is_passable()) {
            // Start moving
            player.move_target = target;
            player.move_progress = 0.0f;
            player.is_moving = true;

            // Set velocity for visual feedback
            transform.velocity = Vec2f{
                static_cast<f32>(player.input_direction.x) / Player::MOVE_DURATION,
                static_cast<f32>(player.input_direction.y) / Player::MOVE_DURATION
            };
        }
    }
}

} // namespace city
