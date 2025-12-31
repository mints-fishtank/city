#pragma once

#include "input_buffer.hpp"
#include "core/ecs/world.hpp"
#include "core/ecs/entity.hpp"

namespace city {

// Entity state for reconciliation
struct EntityState {
    NetEntityId net_id;
    Vec2f position;
    Vec2f velocity;
};

// Client-side prediction system
class PredictionSystem {
public:
    explicit PredictionSystem(World& world);

    void update(f32 dt);

    // Record local input for the current tick
    void record_input(InputSnapshot input);

    // Called when server state is received
    void on_server_state(u32 server_tick, const std::vector<EntityState>& states);

    // Set the local player entity
    void set_local_player(NetEntityId net_id);

    // Get predicted position for interpolation
    Vec2f get_predicted_position(NetEntityId net_id) const;

private:
    void reconcile(u32 server_tick, const EntityState& authoritative_state);
    void apply_input(const InputSnapshot& input, f32 dt);

    World& world_;
    InputBuffer input_buffer_;
    NetEntityId local_player_id_{INVALID_NET_ENTITY_ID};
    u32 last_server_tick_{0};
};

} // namespace city
