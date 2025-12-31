#include "movement.hpp"
#include "core/ecs/world.hpp"
#include "core/game/components/transform.hpp"
#include "core/game/components/player.hpp"

namespace city {

void MovementSystem::update([[maybe_unused]] World& world, [[maybe_unused]] f32 dt) {
    // Grid-based movement is handled by:
    // - Server: InputProcessor::update_grid_movement
    // - Client: PredictionSystem::update_grid_movement
    // This system is kept for potential future use with non-player entities
}

} // namespace city
