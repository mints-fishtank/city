#pragma once

#include "core/ecs/world.hpp"
#include "core/grid/tilemap.hpp"
#include "core/net/message.hpp"
#include <optional>
#include <unordered_map>

namespace city {

// Forward declarations
struct Transform;
struct Player;

class InputProcessor {
public:
    InputProcessor(World& world, TileMap& tilemap);

    // Set the latest input from a client (replaces any previous unprocessed input)
    void set_input(NetEntityId entity, const net::PlayerInputPayload& input);

    // Process all pending inputs
    void update(World& world, f32 dt);

private:
    void update_grid_movement(Transform& transform, Player& player, f32 dt);

    TileMap& tilemap_;
    std::unordered_map<NetEntityId, std::optional<net::PlayerInputPayload>> latest_inputs_;
};

} // namespace city
