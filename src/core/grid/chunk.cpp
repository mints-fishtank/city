#include "chunk.hpp"

namespace city {

Tile* Chunk::at_world(TilePos world_pos) {
    if (!contains(world_pos)) return nullptr;
    TilePos local = world_to_local(world_pos);
    return &at(local.x, local.y);
}

const Tile* Chunk::at_world(TilePos world_pos) const {
    if (!contains(world_pos)) return nullptr;
    TilePos local = world_to_local(world_pos);
    return &at(local.x, local.y);
}

bool Chunk::contains(TilePos world_pos) const {
    return world_pos.x >= origin_.x && world_pos.x < origin_.x + CHUNK_SIZE &&
           world_pos.y >= origin_.y && world_pos.y < origin_.y + CHUNK_SIZE;
}

TilePos Chunk::world_to_local(TilePos world_pos) {
    // Use floor division to handle negative coordinates correctly
    auto floor_div = [](i32 a, i32 b) -> i32 {
        return a >= 0 ? a / b : (a - b + 1) / b;
    };

    i32 chunk_x = floor_div(world_pos.x, CHUNK_SIZE) * CHUNK_SIZE;
    i32 chunk_y = floor_div(world_pos.y, CHUNK_SIZE) * CHUNK_SIZE;

    return {world_pos.x - chunk_x, world_pos.y - chunk_y};
}

TilePos Chunk::get_chunk_origin(TilePos world_pos) {
    auto floor_div = [](i32 a, i32 b) -> i32 {
        return a >= 0 ? a / b : (a - b + 1) / b;
    };

    return {
        floor_div(world_pos.x, CHUNK_SIZE) * CHUNK_SIZE,
        floor_div(world_pos.y, CHUNK_SIZE) * CHUNK_SIZE
    };
}

void Chunk::serialize(Serializer& s) const {
    origin_.serialize(s);
    for (const auto& tile : tiles_) {
        tile.serialize(s);
    }
}

void Chunk::deserialize(Deserializer& d) {
    origin_.deserialize(d);
    for (auto& tile : tiles_) {
        tile.deserialize(d);
    }
}

void Chunk::fill(Tile tile) {
    for (auto& t : tiles_) {
        t = tile;
    }
}

} // namespace city
