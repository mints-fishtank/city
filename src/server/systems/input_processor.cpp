#include "input_processor.hpp"
#include "core/game/components/transform.hpp"
#include "core/game/components/player.hpp"
#include "core/game/systems/movement.hpp"

namespace city {

InputProcessor::InputProcessor([[maybe_unused]] World& world, TileMap& tilemap)
    : tilemap_(tilemap) {}

void InputProcessor::set_input(NetEntityId entity, const net::PlayerInputPayload& input) {
    latest_inputs_[entity] = input;
}

void InputProcessor::update(World& world, f32 dt) {
    // Process the latest input for each entity
    for (auto& [net_id, input_opt] : latest_inputs_) {
        if (!input_opt) continue;

        auto input = *input_opt;
        input_opt = std::nullopt;  // Clear after processing

        Entity entity = world.get_by_net_id(net_id);
        if (!entity.is_valid()) continue;

        auto* player = world.get_component<Player>(entity);
        if (!player) continue;

        // Apply input using shared system
        MoverSystem::apply_input(*player, {input.move_x, input.move_y});
    }

    // Update all players' movement using shared system
    world.each<Transform, Player>([this, dt](Entity, Transform& transform, Player& player) {
        MoverSystem::update_movement(transform, player, tilemap_, dt);
    });
}

} // namespace city
