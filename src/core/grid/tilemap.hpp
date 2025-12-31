#pragma once

#include "chunk.hpp"
#include <unordered_map>
#include <memory>
#include <vector>
#include <optional>

namespace city {

// TileMap manages a collection of chunks
class TileMap {
public:
    TileMap() = default;

    // Map dimensions (in tiles, 0 means unbounded)
    void set_bounds(i32 width, i32 height);
    i32 width() const { return width_; }
    i32 height() const { return height_; }
    bool has_bounds() const { return width_ > 0 && height_ > 0; }

    // Check if position is within bounds
    bool in_bounds(TilePos pos) const;

    // ========== Tile Access ==========

    // Get tile at position (returns nullptr if chunk doesn't exist)
    Tile* get_tile(TilePos pos);
    const Tile* get_tile(TilePos pos) const;

    // Set tile at position (creates chunk if needed)
    void set_tile(TilePos pos, Tile tile);

    // Check if tile is passable
    bool is_passable(TilePos pos) const;

    // Check if tile blocks line of sight
    bool is_opaque(TilePos pos) const;

    // Get passable neighbors (for pathfinding)
    std::vector<TilePos> get_passable_neighbors(TilePos pos, bool allow_diagonal = false) const;

    // ========== Chunk Access ==========

    // Get chunk at origin (returns nullptr if doesn't exist)
    Chunk* get_chunk(TilePos chunk_origin);
    const Chunk* get_chunk(TilePos chunk_origin) const;

    // Get or create chunk
    Chunk& get_or_create_chunk(TilePos chunk_origin);

    // Check if chunk exists
    bool has_chunk(TilePos chunk_origin) const;

    // Get all chunk origins
    std::vector<TilePos> get_chunk_origins() const;

    // ========== Line of Sight ==========

    // Check if there's line of sight between two positions
    bool has_line_of_sight(TilePos from, TilePos to) const;

    // Get all tiles along a line (Bresenham)
    static std::vector<TilePos> get_line(TilePos from, TilePos to);

    // ========== Serialization ==========

    void serialize(Serializer& s) const;
    void deserialize(Deserializer& d);

    // Serialize only visible chunks (for client sync)
    void serialize_region(Serializer& s, Recti region) const;

    // ========== Utilities ==========

    // Clear all chunks
    void clear();

    // Number of loaded chunks
    size_t chunk_count() const { return chunks_.size(); }

private:
    i32 width_{0};
    i32 height_{0};
    std::unordered_map<TilePos, std::unique_ptr<Chunk>> chunks_;
};

} // namespace city
