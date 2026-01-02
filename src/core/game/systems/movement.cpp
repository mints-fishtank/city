#include "movement.hpp"
#include <cmath>

namespace city {
namespace MoverSystem {

void apply_input(Player& player, Vec2i direction) {
    // Store the input direction
    player.input_direction = direction;

    // If currently in a grid-locked move, queue the direction for when move completes
    bool is_moving = (player.movement_mode == MovementMode::GridLocked)
        ? player.is_moving
        : (direction.x != 0 || direction.y != 0);

    if (player.movement_mode == MovementMode::GridLocked && is_moving) {
        player.queued_direction = direction;
    }
}

void update_movement(Transform& transform, Player& player, const TileMap& tilemap, f32 dt) {
    Vec2f move_dir{
        static_cast<f32>(player.input_direction.x),
        static_cast<f32>(player.input_direction.y)
    };

    // Normalize diagonal movement to prevent faster diagonal speed
    if (move_dir.x != 0.0f && move_dir.y != 0.0f) {
        constexpr f32 inv_sqrt2 = 0.7071067811865475f;
        move_dir.x *= inv_sqrt2;
        move_dir.y *= inv_sqrt2;
    }

    // Handle free movement mode - purely position based
    if (player.movement_mode == MovementMode::Free) {
        transform.velocity = Vec2f{
            move_dir.x * Player::FREE_MOVE_SPEED,
            move_dir.y * Player::FREE_MOVE_SPEED
        };
        transform.position.x += transform.velocity.x * dt;
        transform.position.y += transform.velocity.y * dt;

        // Derive grid_pos from position (for collision checks, etc.)
        player.grid_pos = Vec2i{
            static_cast<i32>(std::floor(transform.position.x)),
            static_cast<i32>(std::floor(transform.position.y))
        };
        player.is_moving = (transform.velocity.x != 0.0f || transform.velocity.y != 0.0f);
        return;
    }

    // Grid-locked movement mode - position is still authoritative
    // but movement snaps to tile boundaries

    if (player.is_moving) {
        // Continue moving toward target
        Vec2f target_pos{
            static_cast<f32>(player.move_target.x) + 0.5f,
            static_cast<f32>(player.move_target.y) + 0.5f
        };

        // Move toward target at grid speed
        f32 speed = 1.0f / Player::MOVE_DURATION;
        Vec2f to_target{
            target_pos.x - transform.position.x,
            target_pos.y - transform.position.y
        };
        f32 dist = std::sqrt(to_target.x * to_target.x + to_target.y * to_target.y);

        if (dist <= speed * dt || dist < 0.001f) {
            // Arrived at target - snap to center
            transform.position = target_pos;
            transform.velocity = {0.0f, 0.0f};
            player.grid_pos = player.move_target;
            player.is_moving = false;

            // Apply queued direction if any
            if (player.queued_direction.x != 0 || player.queued_direction.y != 0) {
                player.input_direction = player.queued_direction;
                player.queued_direction = {0, 0};
            }
        } else {
            // Move toward target
            transform.velocity = Vec2f{
                (to_target.x / dist) * speed,
                (to_target.y / dist) * speed
            };
            transform.position.x += transform.velocity.x * dt;
            transform.position.y += transform.velocity.y * dt;
        }
    }

    // Try to start a new move if not currently moving and have input
    if (!player.is_moving && (player.input_direction.x != 0 || player.input_direction.y != 0)) {
        // Derive current grid position from actual position
        player.grid_pos = Vec2i{
            static_cast<i32>(std::floor(transform.position.x)),
            static_cast<i32>(std::floor(transform.position.y))
        };

        Vec2i target{
            player.grid_pos.x + player.input_direction.x,
            player.grid_pos.y + player.input_direction.y
        };

        // Check if target tile is passable
        TilePos tile_pos{target.x, target.y};
        const Tile* tile = tilemap.get_tile(tile_pos);

        if (tile && tile->is_passable()) {
            // Start the move
            player.move_target = target;
            player.is_moving = true;

            // Set initial velocity toward target
            Vec2f target_pos{
                static_cast<f32>(target.x) + 0.5f,
                static_cast<f32>(target.y) + 0.5f
            };
            f32 speed = 1.0f / Player::MOVE_DURATION;
            Vec2f to_target{
                target_pos.x - transform.position.x,
                target_pos.y - transform.position.y
            };
            f32 dist = std::sqrt(to_target.x * to_target.x + to_target.y * to_target.y);
            if (dist > 0.001f) {
                transform.velocity = Vec2f{
                    (to_target.x / dist) * speed,
                    (to_target.y / dist) * speed
                };
            }
        }
    }
}

} // namespace MoverSystem
} // namespace city
