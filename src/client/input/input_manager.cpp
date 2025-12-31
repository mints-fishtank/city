#include "input_manager.hpp"

namespace city {

InputManager::InputManager() {
    load_default_bindings();
}

void InputManager::handle_event(const SDL_Event& event) {
    if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
        auto it = key_bindings_.find(event.key.key);
        if (it != key_bindings_.end()) {
            size_t action_index = static_cast<size_t>(it->second);
            current_state_[action_index] = (event.type == SDL_EVENT_KEY_DOWN);
        }
    } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
        mouse_pos_ = {static_cast<i32>(event.motion.x), static_cast<i32>(event.motion.y)};
    }
}

void InputManager::update() {
    previous_state_ = current_state_;
}

bool InputManager::is_pressed(Action action) const {
    return current_state_[static_cast<size_t>(action)];
}

bool InputManager::just_pressed(Action action) const {
    size_t idx = static_cast<size_t>(action);
    return current_state_[idx] && !previous_state_[idx];
}

bool InputManager::just_released(Action action) const {
    size_t idx = static_cast<size_t>(action);
    return !current_state_[idx] && previous_state_[idx];
}

Vec2i InputManager::get_movement_direction() const {
    Vec2i dir{0, 0};
    if (is_pressed(Action::MoveUp)) dir.y -= 1;
    if (is_pressed(Action::MoveDown)) dir.y += 1;
    if (is_pressed(Action::MoveLeft)) dir.x -= 1;
    if (is_pressed(Action::MoveRight)) dir.x += 1;
    return dir;
}

InputSnapshot InputManager::capture(u32 tick) const {
    Vec2i dir = get_movement_direction();
    return InputSnapshot{
        .tick = tick,
        .move_x = static_cast<i8>(dir.x),
        .move_y = static_cast<i8>(dir.y),
        .interact = is_pressed(Action::Interact),
        .secondary = is_pressed(Action::Secondary),
        .target_tile = {0, 0}  // TODO: Calculate from mouse position
    };
}

Vec2f InputManager::mouse_world_position(Vec2f camera_pos, f32 zoom) const {
    return {
        camera_pos.x + static_cast<f32>(mouse_pos_.x) / zoom,
        camera_pos.y + static_cast<f32>(mouse_pos_.y) / zoom
    };
}

void InputManager::bind_key(SDL_Keycode key, Action action) {
    key_bindings_[key] = action;
}

void InputManager::load_default_bindings() {
    key_bindings_.clear();
    bind_key(SDLK_W, Action::MoveUp);
    bind_key(SDLK_S, Action::MoveDown);
    bind_key(SDLK_A, Action::MoveLeft);
    bind_key(SDLK_D, Action::MoveRight);
    bind_key(SDLK_UP, Action::MoveUp);
    bind_key(SDLK_DOWN, Action::MoveDown);
    bind_key(SDLK_LEFT, Action::MoveLeft);
    bind_key(SDLK_RIGHT, Action::MoveRight);
    bind_key(SDLK_E, Action::Interact);
    bind_key(SDLK_Q, Action::Secondary);
    bind_key(SDLK_RETURN, Action::Chat);
    bind_key(SDLK_TAB, Action::Inventory);
    bind_key(SDLK_ESCAPE, Action::Escape);
}

} // namespace city
