#pragma once

#include "core/util/types.hpp"
#include "core/net/serialization.hpp"
#include <string>

namespace city {

// Player component - marks an entity as a player
struct Player {
    std::string name;                   // Player display name
    u32 session_id{0};                  // Server session ID
    u8 team{0};                         // Team/faction ID
    bool is_local{false};               // True for the local player on client

    // Movement settings
    f32 move_speed{4.0f};               // Tiles per second

    // State
    bool is_moving{false};
    Vec2i input_direction{0, 0};        // Current movement input (-1, 0, or 1 for each axis)

    void serialize(Serializer& s) const {
        s.write_string(name);
        s.write_u32(session_id);
        s.write_u8(team);
        s.write_f32(move_speed);
    }

    void deserialize(Deserializer& d) {
        name = d.read_string();
        session_id = d.read_u32();
        team = d.read_u8();
        move_speed = d.read_f32();
    }
};

// Input snapshot for a single tick
struct InputSnapshot {
    u32 tick{0};                        // Tick this input is for
    i8 move_x{0};                       // -1, 0, or 1
    i8 move_y{0};                       // -1, 0, or 1
    bool interact{false};               // Interact/use button
    bool secondary{false};              // Secondary action button
    Vec2i target_tile{0, 0};            // Mouse target tile position

    void serialize(Serializer& s) const {
        s.write_u32(tick);
        s.write_i8(move_x);
        s.write_i8(move_y);
        u8 buttons = 0;
        if (interact) buttons |= 0x01;
        if (secondary) buttons |= 0x02;
        s.write_u8(buttons);
        s.write_vec2i(target_tile);
    }

    void deserialize(Deserializer& d) {
        tick = d.read_u32();
        move_x = d.read_i8();
        move_y = d.read_i8();
        u8 buttons = d.read_u8();
        interact = (buttons & 0x01) != 0;
        secondary = (buttons & 0x02) != 0;
        target_tile = d.read_vec2i();
    }
};

// Networked player state (sent in updates)
struct PlayerState {
    Vec2f position;
    Vec2f velocity;
    bool is_moving;

    void serialize(Serializer& s) const {
        s.write_vec2f(position);
        s.write_vec2f(velocity);
        s.write_bool(is_moving);
    }

    void deserialize(Deserializer& d) {
        position = d.read_vec2f();
        velocity = d.read_vec2f();
        is_moving = d.read_bool();
    }
};

} // namespace city
