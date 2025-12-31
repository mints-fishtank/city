#pragma once

#include "core/ecs/world.hpp"
#include "core/grid/tilemap.hpp"
#include "core/net/message.hpp"
#include <queue>
#include <unordered_map>

namespace city {

// Forward declarations
struct Transform;
struct Player;

class InputProcessor {
public:
    InputProcessor(World& world, TileMap& tilemap);

    // Queue input from a client
    void queue_input(NetEntityId entity, const net::PlayerInputPayload& input);

    // Process all queued inputs
    void update(World& world, f32 dt);

private:
    void update_grid_movement(Transform& transform, Player& player, f32 dt);

    TileMap& tilemap_;
    std::unordered_map<NetEntityId, std::queue<net::PlayerInputPayload>> input_queues_;
};

} // namespace city
