#include "interpolation.hpp"
#include <algorithm>

namespace city {

void InterpolationSystem::update(f32 dt) {
    for (auto& [net_id, state] : states_) {
        state.interpolation_time += dt / INTERPOLATION_DURATION;
        state.interpolation_time = std::min(state.interpolation_time, 1.0f);
    }
}

void InterpolationSystem::set_target(NetEntityId net_id, Vec2f position) {
    auto it = states_.find(net_id);
    if (it != states_.end()) {
        // Snap previous to current interpolated position before setting new target
        Vec2f current = get_position(net_id, position);
        it->second.previous_position = current;
        it->second.target_position = position;
        it->second.interpolation_time = 0.0f;
    } else {
        // First time seeing this entity - start at target
        states_[net_id] = InterpolationState{
            .previous_position = position,
            .target_position = position,
            .interpolation_time = 1.0f
        };
    }
}

Vec2f InterpolationSystem::get_position(NetEntityId net_id, Vec2f default_pos) const {
    auto it = states_.find(net_id);
    if (it == states_.end()) {
        return default_pos;
    }

    const auto& state = it->second;
    f32 t = state.interpolation_time;

    // Linear interpolation
    return Vec2f{
        state.previous_position.x + (state.target_position.x - state.previous_position.x) * t,
        state.previous_position.y + (state.target_position.y - state.previous_position.y) * t
    };
}

void InterpolationSystem::remove(NetEntityId net_id) {
    states_.erase(net_id);
}

bool InterpolationSystem::is_tracking(NetEntityId net_id) const {
    return states_.find(net_id) != states_.end();
}

} // namespace city
