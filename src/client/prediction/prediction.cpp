#include "prediction.hpp"
#include "core/game/components/transform.hpp"
#include "core/game/components/player.hpp"

namespace city {

PredictionSystem::PredictionSystem(World& world) : world_(world) {}

void PredictionSystem::update(f32 /*dt*/) {
    // Prediction happens in record_input and reconcile
}

void PredictionSystem::record_input(InputSnapshot input) {
    input_buffer_.add(input);

    // Apply input locally for immediate feedback
    Entity player = world_.get_by_net_id(local_player_id_);
    if (!player.is_valid()) return;

    auto* transform = world_.get_component<Transform>(player);
    auto* player_comp = world_.get_component<Player>(player);
    if (!transform || !player_comp) return;

    // Update player input state
    player_comp->input_direction = {input.move_x, input.move_y};
    player_comp->is_moving = (input.move_x != 0 || input.move_y != 0);
}

void PredictionSystem::on_server_state(u32 server_tick, const std::vector<EntityState>& states) {
    // Find local player state
    for (const auto& state : states) {
        if (state.net_id == local_player_id_) {
            reconcile(server_tick, state);
            break;
        }
    }

    // Acknowledge processed inputs
    input_buffer_.acknowledge(server_tick);
    last_server_tick_ = server_tick;
}

void PredictionSystem::set_local_player(NetEntityId net_id) {
    local_player_id_ = net_id;
}

Vec2f PredictionSystem::get_predicted_position(NetEntityId net_id) const {
    Entity e = world_.get_by_net_id(net_id);
    if (!e.is_valid()) return {0.0f, 0.0f};

    const auto* transform = world_.get_component<Transform>(e);
    return transform ? transform->position : Vec2f{0.0f, 0.0f};
}

void PredictionSystem::reconcile(u32 server_tick, const EntityState& authoritative_state) {
    Entity player = world_.get_by_net_id(local_player_id_);
    if (!player.is_valid()) return;

    auto* transform = world_.get_component<Transform>(player);
    if (!transform) return;

    // Check if prediction was accurate
    Vec2f predicted = transform->position;
    Vec2f server = authoritative_state.position;
    f32 error = predicted.distance(server);

    // If error is small, no correction needed
    if (error < 0.01f) return;

    // Snap to server position
    transform->position = server;
    transform->velocity = authoritative_state.velocity;

    // Re-apply unacknowledged inputs
    auto unacked = input_buffer_.get_unacknowledged();
    constexpr f32 dt = 1.0f / 60.0f;  // Fixed timestep

    for (const auto& input : unacked) {
        if (input.tick > server_tick) {
            apply_input(input, dt);
        }
    }
}

void PredictionSystem::apply_input(const InputSnapshot& input, f32 dt) {
    Entity player = world_.get_by_net_id(local_player_id_);
    if (!player.is_valid()) return;

    auto* transform = world_.get_component<Transform>(player);
    auto* player_comp = world_.get_component<Player>(player);
    if (!transform || !player_comp) return;

    // Calculate velocity from input
    Vec2f velocity{
        static_cast<f32>(input.move_x) * player_comp->move_speed,
        static_cast<f32>(input.move_y) * player_comp->move_speed
    };

    // Normalize diagonal movement
    if (velocity.length_squared() > player_comp->move_speed * player_comp->move_speed) {
        velocity = velocity.normalized() * player_comp->move_speed;
    }

    // Apply movement
    transform->position = transform->position + velocity * dt;
}

} // namespace city
