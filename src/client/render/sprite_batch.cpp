#include "sprite_batch.hpp"

namespace city {

void SpriteBatch::begin() {
    batching_ = true;
    sprites_.clear();
}

void SpriteBatch::end() {
    flush();
    batching_ = false;
}

void SpriteBatch::draw(u32 texture_id, Rectf src, Rectf dest, Color tint) {
    sprites_.push_back({texture_id, src, dest, tint});
}

void SpriteBatch::draw(u32 texture_id, Vec2f position, Color tint) {
    // TODO: Get texture dimensions
    Rectf src{0.0f, 0.0f, 1.0f, 1.0f};
    Rectf dest{position.x, position.y, 32.0f, 32.0f};
    draw(texture_id, src, dest, tint);
}

void SpriteBatch::flush() {
    if (sprites_.empty()) return;

    // TODO: Batch render with Vulkan
    sprites_.clear();
}

} // namespace city
