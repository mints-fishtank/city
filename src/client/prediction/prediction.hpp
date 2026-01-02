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
    Vec2i grid_pos;
    Vec2i move_target;
    Vec2i input_direction;  // Current input direction
    bool is_moving;
};

// Client-side prediction system with full resimulation
// Based on SS14's approach: when server state arrives, reset to authoritative
// state and replay all inputs since that tick
class PredictionSystem {
public:
    PredictionSystem(World& world, TileMap& tilemap);

    void update(f32 dt);

    // Record local input for the current tick and apply locally
    void record_input(InputSnapshot input, f32 dt);

    // Called when server state is received - triggers reconciliation
    void on_server_state(u32 server_tick, const std::vector<EntityState>& states);

    // Set the local player entity
    void set_local_player(NetEntityId net_id);

    // Get predicted position for interpolation
    Vec2f get_predicted_position(NetEntityId net_id) const;

    // Get last acknowledged server tick
    u32 last_server_tick() const { return last_server_tick_; }

    // Clear input buffer (used when re-syncing ticks)
    void clear_inputs();

private:
    // Reset to server state and replay all inputs since that tick
    void reconcile(u32 server_tick, const EntityState& authoritative_state);

    // Resimulate a single tick with given input
    void resimulate_tick(const InputSnapshot& input);

    World& world_;
    TileMap& tilemap_;
    InputBuffer input_buffer_;
    NetEntityId local_player_id_{INVALID_NET_ENTITY_ID};
    u32 last_server_tick_{0};

    // Fixed timestep for resimulation (must match server tick rate)
    f32 tick_dt_{1.0f / 60.0f};

    // Smooth visual correction (blends out misprediction over a few frames)
    Vec2f position_error_{0.0f, 0.0f};
    static constexpr f32 ERROR_BLEND_RATE = 20.0f;

    // Estimated round-trip latency in ticks (used for input acknowledgment)
    // Conservative estimate: 100ms = ~6 ticks at 60Hz
    static constexpr u32 RTT_ESTIMATE_TICKS = 6;
};

} // namespace city
