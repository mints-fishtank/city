#pragma once

#include "core/util/types.hpp"

namespace city {

// Forward declaration
class World;

// Base class for systems that operate on entities
class System {
public:
    virtual ~System() = default;

    // Called each tick to update entities
    virtual void update(World& world, f32 dt) = 0;

    // Optional: called when system is added to world
    virtual void on_added(World& /*world*/) {}

    // Optional: called when system is removed from world
    virtual void on_removed(World& /*world*/) {}
};

// System execution context - determines where a system runs
enum class SystemContext : u8 {
    Shared,      // Runs on both client and server
    ServerOnly,  // Only on authoritative server
    ClientOnly,  // Only on client (rendering, prediction, etc.)
};

} // namespace city
