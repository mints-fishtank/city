#pragma once

#include "core/util/types.hpp"
#include <vector>

namespace city {

// Sprite batch for efficient 2D rendering
class SpriteBatch {
public:
    SpriteBatch() = default;

    void begin();
    void end();

    void draw(u32 texture_id, Rectf src, Rectf dest, Color tint = Color::white());
    void draw(u32 texture_id, Vec2f position, Color tint = Color::white());

    void flush();

private:
    struct Sprite {
        u32 texture_id;
        Rectf src;
        Rectf dest;
        Color tint;
    };

    std::vector<Sprite> sprites_;
    bool batching_{false};
};

} // namespace city
