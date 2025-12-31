#pragma once

#include "core/util/types.hpp"
#include "core/net/serialization.hpp"

namespace city {

// Transform component - position and movement state
struct Transform {
    Vec2f position{0.0f, 0.0f};     // World position (can be fractional for smooth movement)
    Vec2f velocity{0.0f, 0.0f};     // Current velocity
    f32 rotation{0.0f};             // Rotation in radians (0 = facing right)

    // Grid position (for tile-based logic)
    Vec2i tile_position() const {
        return {
            static_cast<i32>(std::floor(position.x)),
            static_cast<i32>(std::floor(position.y))
        };
    }

    // Serialization
    void serialize(Serializer& s) const {
        s.write_vec2f(position);
        s.write_vec2f(velocity);
        s.write_f32(rotation);
    }

    void deserialize(Deserializer& d) {
        position = d.read_vec2f();
        velocity = d.read_vec2f();
        rotation = d.read_f32();
    }
};

// Sprite component - visual representation
struct Sprite {
    u32 texture_id{0};              // Resource ID for texture
    u16 sprite_index{0};            // Index in sprite sheet
    Color tint{Color::white()};     // Color tint
    Vec2f scale{1.0f, 1.0f};        // Scale multiplier
    i8 z_order{0};                  // Render order (higher = on top)
    bool visible{true};
    bool flip_x{false};
    bool flip_y{false};

    void serialize(Serializer& s) const {
        s.write_u32(texture_id);
        s.write_u16(sprite_index);
        s.write_u32(tint.to_u32());
        s.write_vec2f(scale);
        s.write_i8(z_order);

        u8 flags = 0;
        if (visible) flags |= 0x01;
        if (flip_x) flags |= 0x02;
        if (flip_y) flags |= 0x04;
        s.write_u8(flags);
    }

    void deserialize(Deserializer& d) {
        texture_id = d.read_u32();
        sprite_index = d.read_u16();
        tint = Color::from_u32(d.read_u32());
        scale = d.read_vec2f();
        z_order = d.read_i8();

        u8 flags = d.read_u8();
        visible = (flags & 0x01) != 0;
        flip_x = (flags & 0x02) != 0;
        flip_y = (flags & 0x04) != 0;
    }
};

// Collision component - physics bounds
struct Collision {
    Vec2f size{1.0f, 1.0f};         // Collision box size
    Vec2f offset{0.0f, 0.0f};       // Offset from transform position
    bool solid{true};               // Blocks movement
    bool trigger{false};            // Triggers events but doesn't block

    // Get collision rectangle at given position
    Rectf bounds_at(Vec2f pos) const {
        return Rectf{
            pos.x + offset.x - size.x / 2.0f,
            pos.y + offset.y - size.y / 2.0f,
            size.x,
            size.y
        };
    }

    void serialize(Serializer& s) const {
        s.write_vec2f(size);
        s.write_vec2f(offset);
        u8 flags = 0;
        if (solid) flags |= 0x01;
        if (trigger) flags |= 0x02;
        s.write_u8(flags);
    }

    void deserialize(Deserializer& d) {
        size = d.read_vec2f();
        offset = d.read_vec2f();
        u8 flags = d.read_u8();
        solid = (flags & 0x01) != 0;
        trigger = (flags & 0x02) != 0;
    }
};

} // namespace city
