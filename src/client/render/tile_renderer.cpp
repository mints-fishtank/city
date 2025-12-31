#include "tile_renderer.hpp"

namespace city {

void TileRenderer::render(const TileMap& tilemap, Rectf view, SpriteBatch& batch) {
    // Calculate visible tile range
    i32 min_x = static_cast<i32>(view.x / tile_size_);
    i32 min_y = static_cast<i32>(view.y / tile_size_);
    i32 max_x = static_cast<i32>((view.x + view.width) / tile_size_) + 1;
    i32 max_y = static_cast<i32>((view.y + view.height) / tile_size_) + 1;

    for (i32 y = min_y; y <= max_y; ++y) {
        for (i32 x = min_x; x <= max_x; ++x) {
            const Tile* tile = tilemap.get_tile({x, y});
            if (!tile) continue;

            Rectf dest{
                static_cast<f32>(x) * tile_size_,
                static_cast<f32>(y) * tile_size_,
                tile_size_,
                tile_size_
            };

            // Draw floor
            if (tile->floor_id != 0) {
                batch.draw(tile->floor_id, {}, dest);
            }

            // Draw wall
            if (tile->wall_id != 0) {
                batch.draw(tile->wall_id, {}, dest);
            }
        }
    }
}

} // namespace city
