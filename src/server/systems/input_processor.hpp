#pragma once

#include "core/ecs/world.hpp"
#include "core/net/message.hpp"
#include <queue>
#include <unordered_map>

namespace city {

class InputProcessor {
public:
    explicit InputProcessor(World& world);

    // Queue input from a client
    void queue_input(NetEntityId entity, const net::PlayerInputPayload& input);

    // Process all queued inputs
    void update(World& world, f32 dt);

private:
    World& world_;
    std::unordered_map<NetEntityId, std::queue<net::PlayerInputPayload>> input_queues_;
};

} // namespace city
