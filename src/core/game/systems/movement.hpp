#pragma once

#include "core/game/components/transform.hpp"
#include "core/game/components/player.hpp"
#include "core/grid/tilemap.hpp"

namespace city {

// Shared movement utilities - used by both client (prediction) and server (authoritative)
// This ensures identical simulation on both sides, which is critical for proper prediction
namespace MoverSystem {

// Process one tick of movement for a player entity
// This is the SINGLE SOURCE OF TRUTH for movement logic
// Both client and server must call this with identical inputs to stay in sync
void update_movement(Transform& transform, Player& player, const TileMap& tilemap, f32 dt);

// Apply input direction to player state
// Call this when new input is received (client: every frame, server: when packet arrives)
void apply_input(Player& player, Vec2i direction);

} // namespace MoverSystem

} // namespace city
