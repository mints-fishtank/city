#pragma once

#include "core/util/types.hpp"
#include "core/net/serialization.hpp"
#include <functional>

namespace city {

// Tile flags
enum class TileFlags : u8 {
    None     = 0,
    Solid    = 1 << 0,  // Blocks movement
    Opaque   = 1 << 1,  // Blocks line of sight
    HasRoof  = 1 << 2,  // Has a roof/ceiling
    Liquid   = 1 << 3,  // Is water/liquid
    Stairs   = 1 << 4,  // Connects to different Z-level
};

inline TileFlags operator|(TileFlags a, TileFlags b) {
    return static_cast<TileFlags>(static_cast<u8>(a) | static_cast<u8>(b));
}

inline TileFlags operator&(TileFlags a, TileFlags b) {
    return static_cast<TileFlags>(static_cast<u8>(a) & static_cast<u8>(b));
}

inline bool has_flag(TileFlags flags, TileFlags flag) {
    return (static_cast<u8>(flags) & static_cast<u8>(flag)) != 0;
}

// Single tile data
struct Tile {
    u16 floor_id{0};        // Floor sprite/type ID
    u16 wall_id{0};         // Wall sprite/type ID (0 = no wall)
    u16 overlay_id{0};      // Overlay sprite (decorations, etc.)
    TileFlags flags{TileFlags::None};

    bool is_passable() const { return !has_flag(flags, TileFlags::Solid); }
    bool is_opaque() const { return has_flag(flags, TileFlags::Opaque); }
    bool has_wall() const { return wall_id != 0; }

    void serialize(Serializer& s) const {
        s.write_u16(floor_id);
        s.write_u16(wall_id);
        s.write_u16(overlay_id);
        s.write_u8(static_cast<u8>(flags));
    }

    void deserialize(Deserializer& d) {
        floor_id = d.read_u16();
        wall_id = d.read_u16();
        overlay_id = d.read_u16();
        flags = static_cast<TileFlags>(d.read_u8());
    }
};

// Tile position (integer grid coordinates)
struct TilePos {
    i32 x{0};
    i32 y{0};

    constexpr TilePos() = default;
    constexpr TilePos(i32 x_, i32 y_) : x(x_), y(y_) {}

    // Convert from world position
    static TilePos from_world(Vec2f world_pos) {
        return {
            static_cast<i32>(std::floor(world_pos.x)),
            static_cast<i32>(std::floor(world_pos.y))
        };
    }

    // Convert to world position (center of tile)
    Vec2f to_world_center() const {
        return {static_cast<f32>(x) + 0.5f, static_cast<f32>(y) + 0.5f};
    }

    // Convert to world position (corner of tile)
    Vec2f to_world() const {
        return {static_cast<f32>(x), static_cast<f32>(y)};
    }

    // Manhattan distance
    i32 manhattan_distance(TilePos other) const {
        return std::abs(x - other.x) + std::abs(y - other.y);
    }

    // Chebyshev distance (diagonal movement allowed)
    i32 chebyshev_distance(TilePos other) const {
        return std::max(std::abs(x - other.x), std::abs(y - other.y));
    }

    // Operators
    TilePos operator+(TilePos other) const { return {x + other.x, y + other.y}; }
    TilePos operator-(TilePos other) const { return {x - other.x, y - other.y}; }
    bool operator==(TilePos other) const { return x == other.x && y == other.y; }
    bool operator!=(TilePos other) const { return !(*this == other); }

    void serialize(Serializer& s) const {
        s.write_i32(x);
        s.write_i32(y);
    }

    void deserialize(Deserializer& d) {
        x = d.read_i32();
        y = d.read_i32();
    }
};

// Cardinal directions
constexpr TilePos DIRECTION_NORTH{0, -1};
constexpr TilePos DIRECTION_SOUTH{0, 1};
constexpr TilePos DIRECTION_EAST{1, 0};
constexpr TilePos DIRECTION_WEST{-1, 0};
constexpr TilePos DIRECTION_NE{1, -1};
constexpr TilePos DIRECTION_NW{-1, -1};
constexpr TilePos DIRECTION_SE{1, 1};
constexpr TilePos DIRECTION_SW{-1, 1};

// 4-directional neighbors
constexpr TilePos CARDINAL_DIRECTIONS[] = {
    DIRECTION_NORTH, DIRECTION_EAST, DIRECTION_SOUTH, DIRECTION_WEST
};

// 8-directional neighbors
constexpr TilePos ALL_DIRECTIONS[] = {
    DIRECTION_NORTH, DIRECTION_NE, DIRECTION_EAST, DIRECTION_SE,
    DIRECTION_SOUTH, DIRECTION_SW, DIRECTION_WEST, DIRECTION_NW
};

} // namespace city

// Hash support for TilePos
template<>
struct std::hash<city::TilePos> {
    size_t operator()(city::TilePos p) const noexcept {
        // Combine x and y into a single hash
        return std::hash<std::int64_t>{}(
            (static_cast<std::int64_t>(p.x) << 32) | static_cast<std::uint32_t>(p.y)
        );
    }
};
