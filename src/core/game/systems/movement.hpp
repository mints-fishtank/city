#pragma once

#include "core/ecs/system.hpp"
#include "core/grid/tilemap.hpp"

namespace city {

// Movement system - processes entity movement
class MovementSystem : public System {
public:
    explicit MovementSystem(const TileMap* tilemap) : tilemap_(tilemap) {}

    void update(World& world, f32 dt) override;

private:
    const TileMap* tilemap_;
};

} // namespace city
