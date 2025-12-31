#include "world.hpp"
#include <stdexcept>

namespace city {

Entity World::create() {
    u32 index;
    u32 generation;

    if (!free_indices_.empty()) {
        // Reuse a freed entity slot
        index = free_indices_.back();
        free_indices_.pop_back();
        generation = generations_[index];
    } else {
        // Allocate a new slot
        index = static_cast<u32>(generations_.size());
        generations_.push_back(0);
        generation = 0;
    }

    ++alive_count_;
    return Entity{index, generation};
}

void World::destroy(Entity e) {
    if (!is_alive(e)) return;

    // Increment generation to invalidate existing references
    ++generations_[e.index];

    // Remove all components
    for (auto& [type, pool] : component_pools_) {
        // We can't easily remove from std::any without knowing the type,
        // so components will be cleaned up lazily when the slot is reused
        // This is acceptable for our use case
    }

    // Remove network ID mapping if present
    auto net_it = entity_to_net_.find(e.index);
    if (net_it != entity_to_net_.end()) {
        net_to_entity_.erase(net_it->second);
        entity_to_net_.erase(net_it);
    }

    // Add to free list for reuse
    free_indices_.push_back(e.index);
    --alive_count_;
}

bool World::is_alive(Entity e) const {
    if (e.index >= generations_.size()) return false;
    return generations_[e.index] == e.generation;
}

void World::assign_net_id(Entity e, NetEntityId net_id) {
    if (!is_alive(e)) return;

    // Remove old mapping if exists
    auto old_it = entity_to_net_.find(e.index);
    if (old_it != entity_to_net_.end()) {
        net_to_entity_.erase(old_it->second);
    }

    // Add new mapping
    net_to_entity_[net_id] = e;
    entity_to_net_[e.index] = net_id;
}

Entity World::get_by_net_id(NetEntityId net_id) const {
    auto it = net_to_entity_.find(net_id);
    if (it == net_to_entity_.end()) {
        return Entity::null();
    }
    return it->second;
}

NetEntityId World::get_net_id(Entity e) const {
    if (!is_alive(e)) return INVALID_NET_ENTITY_ID;

    auto it = entity_to_net_.find(e.index);
    if (it == entity_to_net_.end()) {
        return INVALID_NET_ENTITY_ID;
    }
    return it->second;
}

NetEntityId World::allocate_net_id() {
    return next_net_id_++;
}

void World::update(f32 dt) {
    for (auto& system : systems_) {
        system->update(*this, dt);
    }
}

} // namespace city
