#include "movement.hpp"
#include "core/ecs/world.hpp"
#include "core/game/components/transform.hpp"
#include "core/game/components/player.hpp"

namespace city {

void MovementSystem::update(World& world, f32 dt) {
    // Process all entities with Transform and Player components
    world.each<Transform, Player>([this, dt](Entity /*e*/, Transform& transform, Player& player) {
        if (!player.is_moving) {
            transform.velocity = {0.0f, 0.0f};
            return;
        }

        // Calculate desired velocity from input
        Vec2f desired_velocity{
            static_cast<f32>(player.input_direction.x) * player.move_speed,
            static_cast<f32>(player.input_direction.y) * player.move_speed
        };

        // Normalize diagonal movement
        if (desired_velocity.length_squared() > player.move_speed * player.move_speed) {
            desired_velocity = desired_velocity.normalized() * player.move_speed;
        }

        transform.velocity = desired_velocity;

        // Calculate new position
        Vec2f new_position = transform.position + transform.velocity * dt;

        // Check collision if tilemap is available
        if (tilemap_) {
            TilePos new_tile = TilePos::from_world(new_position);
            if (!tilemap_->is_passable(new_tile)) {
                // Try sliding along walls
                Vec2f slide_x = transform.position + Vec2f{transform.velocity.x * dt, 0.0f};
                Vec2f slide_y = transform.position + Vec2f{0.0f, transform.velocity.y * dt};

                if (tilemap_->is_passable(TilePos::from_world(slide_x))) {
                    new_position = slide_x;
                } else if (tilemap_->is_passable(TilePos::from_world(slide_y))) {
                    new_position = slide_y;
                } else {
                    return;  // Can't move
                }
            }
        }

        transform.position = new_position;
    });
}

} // namespace city
