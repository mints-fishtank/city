#pragma once

#include "core/util/types.hpp"
#include "core/ecs/entity.hpp"
#include <unordered_map>

namespace city {

// Interpolation state for a remote entity
struct InterpolationState {
    Vec2f previous_position{0.0f, 0.0f};
    Vec2f target_position{0.0f, 0.0f};
    f32 interpolation_time{0.0f};  // 0.0 = at previous, 1.0 = at target
};

// System for interpolating remote entity positions
class InterpolationSystem {
public:
    // How long to interpolate between server updates (in seconds)
    static constexpr f32 INTERPOLATION_DURATION = 1.0f / 20.0f;  // ~50ms buffer

    // Update interpolation progress
    void update(f32 dt);

    // Set a new target position for an entity
    void set_target(NetEntityId net_id, Vec2f position);

    // Get interpolated position for an entity
    Vec2f get_position(NetEntityId net_id, Vec2f default_pos) const;

    // Remove an entity from interpolation tracking
    void remove(NetEntityId net_id);

    // Check if an entity is being tracked
    bool is_tracking(NetEntityId net_id) const;

private:
    std::unordered_map<NetEntityId, InterpolationState> states_;
};

} // namespace city
