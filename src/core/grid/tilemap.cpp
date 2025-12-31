#include "tilemap.hpp"
#include <cmath>

namespace city {

void TileMap::set_bounds(i32 width, i32 height) {
    width_ = width;
    height_ = height;
}

bool TileMap::in_bounds(TilePos pos) const {
    if (!has_bounds()) return true;
    return pos.x >= 0 && pos.x < width_ && pos.y >= 0 && pos.y < height_;
}

Tile* TileMap::get_tile(TilePos pos) {
    if (!in_bounds(pos)) return nullptr;

    TilePos chunk_origin = Chunk::get_chunk_origin(pos);
    auto* chunk = get_chunk(chunk_origin);
    if (!chunk) return nullptr;

    return chunk->at_world(pos);
}

const Tile* TileMap::get_tile(TilePos pos) const {
    if (!in_bounds(pos)) return nullptr;

    TilePos chunk_origin = Chunk::get_chunk_origin(pos);
    auto* chunk = get_chunk(chunk_origin);
    if (!chunk) return nullptr;

    return chunk->at_world(pos);
}

void TileMap::set_tile(TilePos pos, Tile tile) {
    if (!in_bounds(pos)) return;

    TilePos chunk_origin = Chunk::get_chunk_origin(pos);
    auto& chunk = get_or_create_chunk(chunk_origin);

    TilePos local = Chunk::world_to_local(pos);
    chunk.at(local.x, local.y) = tile;
}

bool TileMap::is_passable(TilePos pos) const {
    const Tile* tile = get_tile(pos);
    return tile && tile->is_passable();
}

bool TileMap::is_opaque(TilePos pos) const {
    const Tile* tile = get_tile(pos);
    return tile && tile->is_opaque();
}

std::vector<TilePos> TileMap::get_passable_neighbors(TilePos pos, bool allow_diagonal) const {
    std::vector<TilePos> neighbors;
    neighbors.reserve(allow_diagonal ? 8 : 4);

    const TilePos* directions = allow_diagonal ? ALL_DIRECTIONS : CARDINAL_DIRECTIONS;
    size_t count = allow_diagonal ? 8 : 4;

    for (size_t i = 0; i < count; ++i) {
        TilePos neighbor = pos + directions[i];
        if (is_passable(neighbor)) {
            // For diagonal movement, also check that we can move through corners
            if (allow_diagonal && directions[i].x != 0 && directions[i].y != 0) {
                TilePos side_a = pos + TilePos{directions[i].x, 0};
                TilePos side_b = pos + TilePos{0, directions[i].y};
                if (!is_passable(side_a) || !is_passable(side_b)) {
                    continue;  // Can't cut through corners
                }
            }
            neighbors.push_back(neighbor);
        }
    }

    return neighbors;
}

Chunk* TileMap::get_chunk(TilePos chunk_origin) {
    auto it = chunks_.find(chunk_origin);
    return it != chunks_.end() ? it->second.get() : nullptr;
}

const Chunk* TileMap::get_chunk(TilePos chunk_origin) const {
    auto it = chunks_.find(chunk_origin);
    return it != chunks_.end() ? it->second.get() : nullptr;
}

Chunk& TileMap::get_or_create_chunk(TilePos chunk_origin) {
    auto it = chunks_.find(chunk_origin);
    if (it != chunks_.end()) {
        return *it->second;
    }

    auto chunk = std::make_unique<Chunk>(chunk_origin);
    auto& ref = *chunk;
    chunks_[chunk_origin] = std::move(chunk);
    return ref;
}

bool TileMap::has_chunk(TilePos chunk_origin) const {
    return chunks_.find(chunk_origin) != chunks_.end();
}

std::vector<TilePos> TileMap::get_chunk_origins() const {
    std::vector<TilePos> origins;
    origins.reserve(chunks_.size());
    for (const auto& [origin, _] : chunks_) {
        origins.push_back(origin);
    }
    return origins;
}

bool TileMap::has_line_of_sight(TilePos from, TilePos to) const {
    auto line = get_line(from, to);
    for (const auto& pos : line) {
        if (pos == from || pos == to) continue;
        if (is_opaque(pos)) return false;
    }
    return true;
}

std::vector<TilePos> TileMap::get_line(TilePos from, TilePos to) {
    std::vector<TilePos> line;

    i32 dx = std::abs(to.x - from.x);
    i32 dy = std::abs(to.y - from.y);
    i32 sx = from.x < to.x ? 1 : -1;
    i32 sy = from.y < to.y ? 1 : -1;
    i32 err = dx - dy;

    TilePos current = from;
    while (true) {
        line.push_back(current);
        if (current == to) break;

        i32 e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            current.x += sx;
        }
        if (e2 < dx) {
            err += dx;
            current.y += sy;
        }
    }

    return line;
}

void TileMap::serialize(Serializer& s) const {
    s.write_i32(width_);
    s.write_i32(height_);
    s.write_u32(static_cast<u32>(chunks_.size()));

    for (const auto& [origin, chunk] : chunks_) {
        chunk->serialize(s);
    }
}

void TileMap::deserialize(Deserializer& d) {
    width_ = d.read_i32();
    height_ = d.read_i32();
    u32 chunk_count = d.read_u32();

    chunks_.clear();
    for (u32 i = 0; i < chunk_count; ++i) {
        auto chunk = std::make_unique<Chunk>();
        chunk->deserialize(d);
        chunks_[chunk->origin()] = std::move(chunk);
    }
}

void TileMap::serialize_region(Serializer& s, Recti region) const {
    // Find chunks that intersect with the region
    std::vector<const Chunk*> visible_chunks;

    TilePos min_chunk = Chunk::get_chunk_origin({region.x, region.y});
    TilePos max_chunk = Chunk::get_chunk_origin({region.x + region.width - 1, region.y + region.height - 1});

    for (i32 cy = min_chunk.y; cy <= max_chunk.y; cy += CHUNK_SIZE) {
        for (i32 cx = min_chunk.x; cx <= max_chunk.x; cx += CHUNK_SIZE) {
            if (auto* chunk = get_chunk({cx, cy})) {
                visible_chunks.push_back(chunk);
            }
        }
    }

    s.write_u32(static_cast<u32>(visible_chunks.size()));
    for (const auto* chunk : visible_chunks) {
        chunk->serialize(s);
    }
}

void TileMap::clear() {
    chunks_.clear();
}

} // namespace city
