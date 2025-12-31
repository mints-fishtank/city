#pragma once

#include "input_buffer.hpp"
#include "core/ecs/world.hpp"
#include "core/ecs/entity.hpp"
#include "core/grid/tilemap.hpp"

namespace city {

// Forward declarations
struct Transform;
struct Player;

// Entity state for reconciliation
struct EntityState {
    NetEntityId net_id;
    Vec2f position;
    Vec2f velocity;
};

// Client-side prediction system
class PredictionSystem {
public:
    PredictionSystem(World& world, TileMap& tilemap);

    void update(f32 dt);

    // Record local input for the current tick and apply locally
    void record_input(InputSnapshot input, f32 dt);

    // Called when server state is received
    void on_server_state(u32 server_tick, const std::vector<EntityState>& states);

    // Set the local player entity
    void set_local_player(NetEntityId net_id);

    // Get predicted position for interpolation
    Vec2f get_predicted_position(NetEntityId net_id) const;

    // Get last acknowledged server tick
    u32 last_server_tick() const { return last_server_tick_; }

private:
    void reconcile(u32 server_tick, const EntityState& authoritative_state);
    void update_grid_movement(Transform& transform, Player& player, f32 dt);

    World& world_;
    TileMap& tilemap_;
    InputBuffer input_buffer_;
    NetEntityId local_player_id_{INVALID_NET_ENTITY_ID};
    u32 last_server_tick_{0};
};

} // namespace city
