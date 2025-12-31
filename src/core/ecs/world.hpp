#pragma once

#include "entity.hpp"
#include "component.hpp"
#include "system.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <any>
#include <typeindex>

namespace city {

// The World manages all entities, components, and systems
class World {
public:
    World() = default;
    ~World() = default;

    // Non-copyable, movable
    World(const World&) = delete;
    World& operator=(const World&) = delete;
    World(World&&) = default;
    World& operator=(World&&) = default;

    // ========== Entity Management ==========

    // Create a new entity
    Entity create();

    // Destroy an entity and all its components
    void destroy(Entity e);

    // Check if an entity is still alive
    bool is_alive(Entity e) const;

    // Get number of living entities
    size_t entity_count() const { return alive_count_; }

    // ========== Component Management ==========

    // Add a component to an entity
    template<typename T>
    T& add_component(Entity e, T component = T{}) {
        auto& pool = get_or_create_pool<T>();
        return pool.set(e.index, std::move(component));
    }

    // Get a component from an entity (returns nullptr if not present)
    template<typename T>
    T* get_component(Entity e) {
        if (!is_alive(e)) return nullptr;
        auto* pool = get_pool<T>();
        if (!pool) return nullptr;
        return pool->get(e.index);
    }

    template<typename T>
    const T* get_component(Entity e) const {
        if (!is_alive(e)) return nullptr;
        auto* pool = get_pool<T>();
        if (!pool) return nullptr;
        return pool->get(e.index);
    }

    // Check if entity has a component
    template<typename T>
    bool has_component(Entity e) const {
        if (!is_alive(e)) return false;
        auto* pool = get_pool<T>();
        return pool && pool->has(e.index);
    }

    // Remove a component from an entity
    template<typename T>
    void remove_component(Entity e) {
        if (!is_alive(e)) return;
        auto* pool = get_pool<T>();
        if (pool) {
            pool->remove(e.index);
        }
    }

    // Get the component pool for iteration
    template<typename T>
    ComponentPool<T>* get_pool() {
        auto it = component_pools_.find(std::type_index(typeid(T)));
        if (it == component_pools_.end()) return nullptr;
        return std::any_cast<ComponentPool<T>>(&it->second);
    }

    template<typename T>
    const ComponentPool<T>* get_pool() const {
        auto it = component_pools_.find(std::type_index(typeid(T)));
        if (it == component_pools_.end()) return nullptr;
        return std::any_cast<ComponentPool<T>>(&it->second);
    }

    // ========== Network Entity ID Mapping ==========

    // Assign a network ID to an entity (server does this)
    void assign_net_id(Entity e, NetEntityId net_id);

    // Get entity by network ID
    Entity get_by_net_id(NetEntityId net_id) const;

    // Get network ID for entity (returns INVALID_NET_ENTITY_ID if not assigned)
    NetEntityId get_net_id(Entity e) const;

    // Allocate next network ID (server only)
    NetEntityId allocate_net_id();

    // ========== System Management ==========

    // Add a system
    template<typename T, typename... Args>
    T& add_system(Args&&... args) {
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *system;
        system->on_added(*this);
        systems_.push_back(std::move(system));
        return ref;
    }

    // Update all systems
    void update(f32 dt);

    // ========== Iteration Helpers ==========

    // Iterate over all entities with specific components
    template<typename First, typename... Rest, typename Func>
    void each(Func&& func) {
        // Use the first component's pool for iteration
        auto* first_pool = get_pool<First>();
        if (!first_pool) return;

        for (auto [entity_index, _] : *first_pool) {
            Entity e{entity_index, generations_[entity_index]};
            if (!is_alive(e)) continue;

            // Check all required components (first is guaranteed, check rest)
            if constexpr (sizeof...(Rest) == 0) {
                func(e, *get_component<First>(e));
            } else {
                if ((has_component<Rest>(e) && ...)) {
                    func(e, *get_component<First>(e), *get_component<Rest>(e)...);
                }
            }
        }
    }

private:
    template<typename T>
    ComponentPool<T>& get_or_create_pool() {
        auto type = std::type_index(typeid(T));
        auto it = component_pools_.find(type);
        if (it == component_pools_.end()) {
            it = component_pools_.emplace(type, ComponentPool<T>{}).first;
        }
        return std::any_cast<ComponentPool<T>&>(it->second);
    }

    // Entity storage
    std::vector<u32> generations_;          // Generation for each entity slot
    std::vector<u32> free_indices_;         // Recycled entity indices
    size_t alive_count_ = 0;

    // Component storage
    std::unordered_map<std::type_index, std::any> component_pools_;

    // Network ID mapping
    std::unordered_map<NetEntityId, Entity> net_to_entity_;
    std::unordered_map<u32, NetEntityId> entity_to_net_;  // entity.index -> net_id
    NetEntityId next_net_id_ = 1;

    // Systems
    std::vector<std::unique_ptr<System>> systems_;
};

} // namespace city
