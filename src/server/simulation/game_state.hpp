#pragma once

#include "core/ecs/world.hpp"
#include "core/grid/tilemap.hpp"

namespace city {

// Manages the authoritative game state
class GameState {
public:
    GameState(World& world, TileMap& tilemap);

    // Serialize full state for new clients
    void serialize_full(Serializer& s) const;

    // Serialize delta since last tick
    void serialize_delta(Serializer& s, u32 since_tick) const;

private:
    World& world_;
    TileMap& tilemap_;
};

} // namespace city
