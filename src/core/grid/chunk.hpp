#pragma once

#include "tile.hpp"
#include <array>

namespace city {

// Chunk size in tiles
constexpr i32 CHUNK_SIZE = 16;

// A chunk is a CHUNK_SIZE x CHUNK_SIZE block of tiles
class Chunk {
public:
    Chunk() = default;
    explicit Chunk(TilePos origin) : origin_(origin) {}

    // Get chunk origin (bottom-left corner in tile coordinates)
    TilePos origin() const { return origin_; }

    // Access tiles by local coordinates (0 to CHUNK_SIZE-1)
    Tile& at(i32 local_x, i32 local_y) {
        return tiles_[static_cast<size_t>(local_y * CHUNK_SIZE + local_x)];
    }

    const Tile& at(i32 local_x, i32 local_y) const {
        return tiles_[static_cast<size_t>(local_y * CHUNK_SIZE + local_x)];
    }

    // Access by world tile position
    Tile* at_world(TilePos world_pos);
    const Tile* at_world(TilePos world_pos) const;

    // Check if world position is within this chunk
    bool contains(TilePos world_pos) const;

    // Convert world position to local coordinates
    static TilePos world_to_local(TilePos world_pos);

    // Get chunk origin from any world position
    static TilePos get_chunk_origin(TilePos world_pos);

    // Serialization
    void serialize(Serializer& s) const;
    void deserialize(Deserializer& d);

    // Fill all tiles with a specific tile
    void fill(Tile tile);

private:
    TilePos origin_{0, 0};
    std::array<Tile, CHUNK_SIZE * CHUNK_SIZE> tiles_{};
};

} // namespace city
