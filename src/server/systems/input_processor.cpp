#include "input_processor.hpp"
#include "core/game/components/transform.hpp"
#include "core/game/components/player.hpp"

namespace city {

InputProcessor::InputProcessor(World& world) : world_(world) {}

void InputProcessor::queue_input(NetEntityId entity, const net::PlayerInputPayload& input) {
    input_queues_[entity].push(input);
}

void InputProcessor::update(World& world, f32 /*dt*/) {
    // Process one input per entity per tick
    for (auto& [net_id, queue] : input_queues_) {
        if (queue.empty()) continue;

        auto input = queue.front();
        queue.pop();

        Entity entity = world.get_by_net_id(net_id);
        if (!entity.is_valid()) continue;

        auto* player = world.get_component<Player>(entity);
        if (!player) continue;

        // Update player input state
        player->input_direction = {input.move_x, input.move_y};
        player->is_moving = (input.move_x != 0 || input.move_y != 0);
    }
}

} // namespace city
