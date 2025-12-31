#pragma once

#include "core/util/types.hpp"
#include <limits>
#include <functional>

namespace city {

// Entity identifier with generation for safe reuse
struct Entity {
    u32 index;
    u32 generation;

    static constexpr Entity null() {
        return {std::numeric_limits<u32>::max(), 0};
    }

    [[nodiscard]] bool is_valid() const { return index != null().index; }

    bool operator==(Entity other) const {
        return index == other.index && generation == other.generation;
    }

    bool operator!=(Entity other) const { return !(*this == other); }
};

// Network-stable entity ID (assigned by server, same across all clients)
using NetEntityId = u32;
constexpr NetEntityId INVALID_NET_ENTITY_ID = 0;

} // namespace city

// Hash support for Entity
template<>
struct std::hash<city::Entity> {
    size_t operator()(city::Entity e) const noexcept {
        return std::hash<std::uint64_t>{}(
            (static_cast<std::uint64_t>(e.generation) << 32) | e.index
        );
    }
};
