#include "game_state.hpp"
#include "core/game/components/transform.hpp"
#include "core/game/components/player.hpp"

namespace city {

GameState::GameState(World& world, TileMap& tilemap)
    : world_(world), tilemap_(tilemap) {}

void GameState::serialize_full(Serializer& s) const {
    // Serialize tilemap
    tilemap_.serialize(s);

    // Count entities with network IDs
    u32 entity_count = 0;
    world_.each<Transform>([&entity_count](Entity e, Transform&) {
        (void)e;
        ++entity_count;
    });

    s.write_u32(entity_count);

    // Serialize each entity
    world_.each<Transform>([this, &s](Entity e, Transform& transform) {
        NetEntityId net_id = world_.get_net_id(e);
        if (net_id == INVALID_NET_ENTITY_ID) return;

        s.write_u32(net_id);

        // Component presence flags
        u8 components = 0;
        if (world_.has_component<Transform>(e)) components |= 0x01;
        if (world_.has_component<Player>(e)) components |= 0x02;
        s.write_u8(components);

        // Serialize present components
        if (components & 0x01) {
            transform.serialize(s);
        }
        if (components & 0x02) {
            world_.get_component<Player>(e)->serialize(s);
        }
    });
}

void GameState::serialize_delta(Serializer& s, u32 /*since_tick*/) const {
    // TODO: Track dirty components and only serialize changes
    // For now, just serialize all entity positions
    u32 count = 0;
    world_.each<Transform>([&count](Entity, Transform&) { ++count; });

    s.write_u32(count);

    world_.each<Transform>([this, &s](Entity e, Transform& transform) {
        NetEntityId net_id = world_.get_net_id(e);
        if (net_id == INVALID_NET_ENTITY_ID) return;

        s.write_u32(net_id);
        s.write_vec2f(transform.position);
        s.write_vec2f(transform.velocity);
    });
}

} // namespace city
