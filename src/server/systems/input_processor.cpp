#include "input_processor.hpp"
#include "core/game/components/transform.hpp"
#include "core/game/components/player.hpp"

namespace city {

InputProcessor::InputProcessor([[maybe_unused]] World& world, TileMap& tilemap)
    : tilemap_(tilemap) {}

void InputProcessor::queue_input(NetEntityId entity, const net::PlayerInputPayload& input) {
    input_queues_[entity].push(input);
}

void InputProcessor::update(World& world, f32 dt) {
    // Process one input per entity per tick
    for (auto& [net_id, queue] : input_queues_) {
        if (queue.empty()) continue;

        auto input = queue.front();
        queue.pop();

        Entity entity = world.get_by_net_id(net_id);
        if (!entity.is_valid()) continue;

        auto* player = world.get_component<Player>(entity);
        auto* transform = world.get_component<Transform>(entity);
        if (!player || !transform) continue;

        // Store input direction
        player->input_direction = {input.move_x, input.move_y};

        // If currently moving, queue the direction for when move completes
        if (player->is_moving) {
            // Always update queued direction (including clearing it when key released)
            player->queued_direction = {input.move_x, input.move_y};
        }
    }

    // Update all players' grid movement
    world.each<Transform, Player>([this, dt](Entity, Transform& transform, Player& player) {
        update_grid_movement(transform, player, dt);
    });
}

void InputProcessor::update_grid_movement(Transform& transform, Player& player, f32 dt) {
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
