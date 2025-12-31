#pragma once

#include "core/util/types.hpp"
#include "core/game/components/player.hpp"
#include <SDL3/SDL.h>
#include <unordered_map>
#include <bitset>

namespace city {

// Game actions (abstracted from keys)
enum class Action : u8 {
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    Interact,
    Secondary,
    Chat,
    Inventory,
    Escape,
    Count
};

class InputManager {
public:
    InputManager();

    void handle_event(const SDL_Event& event);
    void update();

    bool is_pressed(Action action) const;
    bool just_pressed(Action action) const;
    bool just_released(Action action) const;

    // Get current movement direction
    Vec2i get_movement_direction() const;

    // Capture input snapshot for network
    InputSnapshot capture(u32 tick) const;

    // Mouse position
    Vec2i mouse_screen_position() const { return mouse_pos_; }
    Vec2f mouse_world_position(Vec2f camera_pos, f32 zoom) const;

    // Keybinding
    void bind_key(SDL_Keycode key, Action action);
    void load_default_bindings();

private:
    static constexpr size_t ACTION_COUNT = static_cast<size_t>(Action::Count);

    std::bitset<ACTION_COUNT> current_state_;
    std::bitset<ACTION_COUNT> previous_state_;
    std::unordered_map<SDL_Keycode, Action> key_bindings_;
    Vec2i mouse_pos_{0, 0};
};

} // namespace city
