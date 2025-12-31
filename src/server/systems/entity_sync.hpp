#pragma once

#include "core/ecs/world.hpp"
#include "../net/server_connection.hpp"

namespace city {

class EntitySync {
public:
    explicit EntitySync(World& world);

    // Broadcast state to all clients
    void broadcast(ServerConnection& connection, u32 tick);

    // Send full state to a specific client
    void send_full_state(ClientSession& session, u32 tick);

private:
    World& world_;
};

} // namespace city
