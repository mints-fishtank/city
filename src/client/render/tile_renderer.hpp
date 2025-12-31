#pragma once

#include "core/grid/tilemap.hpp"
#include "sprite_batch.hpp"

namespace city {

class TileRenderer {
public:
    TileRenderer() = default;

    void render(const TileMap& tilemap, Rectf view, SpriteBatch& batch);

    void set_tile_size(f32 size) { tile_size_ = size; }
    f32 tile_size() const { return tile_size_; }

private:
    f32 tile_size_{32.0f};
};

} // namespace city
