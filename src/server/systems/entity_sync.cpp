#include "entity_sync.hpp"
#include "core/game/components/transform.hpp"
#include "core/game/components/player.hpp"

namespace city {

EntitySync::EntitySync(World& world) : world_(world) {}

void EntitySync::broadcast(ServerConnection& connection, u32 tick) {
    // Build delta state message
    Serializer s;
    s.write_u32(tick);

    // Count entities with valid net IDs
    u32 count = 0;
    world_.each<Transform>([this, &count](Entity e, Transform&) {
        if (world_.get_net_id(e) != INVALID_NET_ENTITY_ID) ++count;
    });
    s.write_u32(count);

    // Serialize entity states
    world_.each<Transform>([this, &s](Entity e, Transform& transform) {
        NetEntityId net_id = world_.get_net_id(e);
        if (net_id == INVALID_NET_ENTITY_ID) return;

        s.write_u32(net_id);
        s.write_vec2f(transform.position);
        s.write_vec2f(transform.velocity);

        // Sync player-specific state
        auto* player = world_.get_component<Player>(e);
        s.write_bool(player != nullptr);
        if (player) {
            s.write_bool(player->is_moving);
            s.write_vec2i(player->grid_pos);
            s.write_vec2i(player->move_target);
        }
    });

    connection.broadcast(
        net::Message{net::MessageType::DeltaState, s.take()},
        net::Reliability::UnreliableSequenced
    );
}

void EntitySync::send_full_state(ClientSession& session, u32 tick) {
    Serializer s;
    s.write_u32(tick);

    // Count entities with valid net IDs
    u32 count = 0;
    world_.each<Transform>([this, &count](Entity e, Transform&) {
        if (world_.get_net_id(e) != INVALID_NET_ENTITY_ID) ++count;
    });
    s.write_u32(count);

    world_.each<Transform>([this, &s](Entity e, Transform& transform) {
        NetEntityId net_id = world_.get_net_id(e);
        if (net_id == INVALID_NET_ENTITY_ID) return;

        s.write_u32(net_id);

        // Component flags
        u8 flags = 0;
        if (world_.has_component<Transform>(e)) flags |= 0x01;
        if (world_.has_component<Player>(e)) flags |= 0x02;
        s.write_u8(flags);

        if (flags & 0x01) {
            transform.serialize(s);
        }

        if (flags & 0x02) {
            world_.get_component<Player>(e)->serialize(s);
        }
    });

    session.send(
        net::Message{net::MessageType::FullState, s.take()},
        net::Reliability::ReliableOrdered
    );
}

} // namespace city
