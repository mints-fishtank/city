# City - Development Progress

This document tracks implementation progress and provides guidance for continuing development.

## Current Status: Phase 4 Complete (Player Movement)

**Last Updated**: December 2025

## Implementation Phases

### Phase 1: Foundation ✅ COMPLETE

Core infrastructure is in place. The project compiles and has the basic architecture.

| Task | Status | Files |
|------|--------|-------|
| Project structure | ✅ | CMakeLists.txt, directory layout |
| Third-party deps | ✅ | third_party/CMakeLists.txt |
| Core types | ✅ | src/core/util/types.hpp |
| Serialization | ✅ | src/core/net/serialization.hpp |
| ECS | ✅ | src/core/ecs/*.hpp |
| Grid system | ✅ | src/core/grid/*.hpp |
| Protocol defs | ✅ | src/core/net/protocol.hpp |
| Message types | ✅ | src/core/net/message.hpp |
| Components | ✅ | src/core/game/components/*.hpp |
| Movement system | ✅ | src/core/game/systems/movement.hpp |
| Client scaffold | ✅ | src/client/*.cpp |
| Server scaffold | ✅ | src/server/*.cpp |
| Unit tests | ✅ | tests/core/*.cpp |

### Phase 2: Networking Core ✅ COMPLETE

Client and server communicate with full entity sync.

| Task | Status | Priority | Description |
|------|--------|----------|-------------|
| Server startup | ✅ | HIGH | ENet host initialization, listening |
| Client connect | ✅ | HIGH | Connect to server, handshake |
| Hello exchange | ✅ | HIGH | Protocol version validation |
| Session management | ✅ | HIGH | Track connected clients |
| Player spawn | ✅ | HIGH | Create entity on connect |
| Input transmission | ✅ | HIGH | Send PlayerInput messages |
| State broadcast | ✅ | HIGH | Send DeltaState to clients |
| Entity spawn/despawn | ✅ | HIGH | Notify clients of player join/leave |
| Disconnect handling | ✅ | MEDIUM | Clean up on disconnect |
| Ping/pong | 🔲 | LOW | Latency measurement |

**Key Files Modified:**
- `src/server/net/server_connection.cpp` - Connection handling
- `src/client/net/client_connection.cpp` - Connect logic
- `src/server/server.cpp` - Message dispatch, spawn/despawn broadcasts
- `src/client/client.cpp` - Message handling, remote entity creation
- `src/server/systems/entity_sync.cpp` - Position broadcast
- `src/core/net/message.hpp` - EntitySpawn/Despawn payloads

**Testing:**
```bash
# Terminal 1: Start server
./run.sh server

# Terminal 2: Start client
./run.sh connect Alice

# Terminal 3: Start another client
./run.sh connect Bob
```

### Phase 3: Rendering ⚡ IN PROGRESS

Using SDL3 2D renderer for development. Vulkan deferred.

| Task | Status | Priority | Description |
|------|--------|----------|-------------|
| SDL2D renderer | ✅ | HIGH | Basic 2D rendering with SDL3 |
| Tile rendering | ✅ | HIGH | Render tilemap with grid |
| Entity rendering | ✅ | HIGH | Render player entities |
| Camera system | ✅ | HIGH | Follow player, zoom in/out |
| Vulkan instance | 🔲 | LOW | Create VkInstance (deferred) |
| Sprite batching | 🔲 | LOW | Efficient sprite rendering |
| Texture loading | 🔲 | MEDIUM | Load sprites from files |

**Current State:**
The client uses SDL3's built-in 2D renderer for quick iteration. This is sufficient for gameplay development. Vulkan can be added later for performance.

**Key Files:**
- `src/client/render/renderer.cpp` - SDL2D rendering
- `src/client/render/renderer.hpp` - Renderer interface

### Phase 4: Player Movement ✅ COMPLETE

Grid-locked tile movement with smooth animation (like SS13).

| Task | Status | Priority | Description |
|------|--------|----------|-------------|
| Grid-locked movement | ✅ | HIGH | Players move tile-by-tile |
| Tile collision | ✅ | HIGH | Can't move into walls |
| Smooth animation | ✅ | HIGH | 0.15s animated transition between tiles |
| Input queuing | ✅ | HIGH | Queue next move during current move |
| Server authority | ✅ | HIGH | Server validates and applies moves |
| Client prediction | ✅ | HIGH | Predict local movement |
| Reconciliation | ✅ | HIGH | Correct prediction errors (>0.5 tile) |
| Interpolation | ✅ | MEDIUM | Smooth remote players |

**Movement System:**
- Players occupy tile centers (position = tile + 0.5)
- Press direction to move one tile (0.15 second animation)
- Can queue next direction while moving
- Server validates collision, client predicts same

**Key Files:**
- `src/core/game/components/player.hpp` - Grid state (grid_pos, move_target, move_progress)
- `src/server/systems/input_processor.cpp` - Server grid movement
- `src/client/prediction/prediction.cpp` - Client grid movement prediction
- `src/client/prediction/interpolation.cpp` - Remote player interpolation

### Phase 5: Entity Sync ✅ COMPLETE

Full entity synchronization working.

| Task | Status | Priority | Description |
|------|--------|----------|-------------|
| Entity spawn broadcast | ✅ | HIGH | Notify clients of new entities |
| Entity despawn broadcast | ✅ | HIGH | Notify clients of removed entities |
| Delta broadcast | ✅ | HIGH | Send position changes per tick |
| Full state send | 🔲 | MEDIUM | Send all entities on connect (optional) |
| Interest management | 🔲 | LOW | Only sync nearby entities |

### Phase 6: Chat System 🔲 NOT STARTED

| Task | Status | Priority | Description |
|------|--------|----------|-------------|
| Chat messages | 🔲 | MEDIUM | Send/receive chat |
| Chat UI | 🔲 | MEDIUM | Display messages |
| Chat input | 🔲 | MEDIUM | Type and submit |
| Channels | 🔲 | LOW | Global, local, team |

### Phase 7: Content System 🔲 NOT STARTED

| Task | Status | Priority | Description |
|------|--------|----------|-------------|
| Manifest generation | 🔲 | MEDIUM | Scan content directory |
| Content packaging | 🔲 | MEDIUM | Pack assets |
| Content transfer | 🔲 | MEDIUM | Stream to clients |
| Content caching | 🔲 | MEDIUM | Cache on client |
| Hot reload | 🔲 | LOW | Reload content without restart |

### Phase 8: Game Features 🔲 NOT STARTED

| Task | Status | Priority | Description |
|------|--------|----------|-------------|
| Round system | 🔲 | MEDIUM | Start/end rounds |
| Spawn points | 🔲 | MEDIUM | Player spawn locations |
| Map loading | 🔲 | MEDIUM | Load maps from files |
| Map editor | 🔲 | LOW | Tool to create maps |

## File Overview

### Core Files (src/core/)

| File | Purpose | Status |
|------|---------|--------|
| `ecs/entity.hpp` | Entity ID with generation | ✅ Complete |
| `ecs/component.hpp` | ComponentPool sparse set | ✅ Complete |
| `ecs/world.hpp` | Entity manager | ✅ Complete |
| `ecs/system.hpp` | System base class | ✅ Complete |
| `grid/tile.hpp` | Tile struct, TilePos | ✅ Complete |
| `grid/chunk.hpp` | 16x16 tile chunk | ✅ Complete |
| `grid/tilemap.hpp` | Chunk manager | ✅ Complete |
| `net/protocol.hpp` | Message types, constants | ✅ Complete |
| `net/serialization.hpp` | Binary serialize/deserialize | ✅ Complete |
| `net/message.hpp` | Message class, payloads | ✅ Complete |
| `content/content_manifest.hpp` | Asset manifest | ✅ Basic |
| `content/content_loader.hpp` | Asset loading | ✅ Stub |
| `game/components/transform.hpp` | Position, velocity | ✅ Complete |
| `game/components/player.hpp` | Player data, input | ✅ Complete |
| `game/systems/movement.hpp` | Movement system | ✅ Basic |

### Client Files (src/client/)

| File | Purpose | Status |
|------|---------|--------|
| `main.cpp` | Entry point | ✅ Complete |
| `client.hpp/cpp` | Main client class | ✅ Scaffold |
| `render/renderer.cpp` | Render loop | ⚠️ Stub |
| `render/vulkan/vk_context.cpp` | Vulkan setup | ⚠️ Stub |
| `input/input_manager.cpp` | Input handling | ✅ Complete |
| `prediction/prediction.cpp` | Client prediction | ✅ Basic |
| `net/client_connection.cpp` | Server connection | ✅ Basic |
| `ui/chat_window.cpp` | Chat UI | ✅ Basic |

### Server Files (src/server/)

| File | Purpose | Status |
|------|---------|--------|
| `main.cpp` | Entry point | ✅ Complete |
| `server.hpp/cpp` | Main server class | ✅ Scaffold |
| `simulation/game_state.cpp` | State serialization | ✅ Basic |
| `simulation/round_manager.cpp` | Round lifecycle | ✅ Basic |
| `net/server_connection.cpp` | Client management | ✅ Basic |
| `net/client_session.cpp` | Per-client state | ✅ Basic |
| `systems/input_processor.cpp` | Process inputs | ✅ Basic |
| `systems/entity_sync.cpp` | State broadcast | ✅ Basic |

## How to Continue Development

### Step 1: Build the Project

```bash
# First time setup
cmake --preset debug

# Build
cmake --build --preset debug

# Run tests
ctest --preset debug
```

### Step 2: Choose a Phase

Look at the phase list above and pick the next incomplete phase. Generally follow the order:
1. Networking Core (get client/server talking)
2. Rendering (see something on screen)
3. Player Movement (move around)
4. Entity Sync (see other players)
5. Chat (communicate)
6. Content System (customization)

### Step 3: Implementation Pattern

For each feature:

1. **Read existing code** - Understand the patterns used
2. **Write tests first** - Add to `tests/core/test_*.cpp`
3. **Implement in core** - Shared logic in `src/core/`
4. **Add to server** - Server-side in `src/server/`
5. **Add to client** - Client-side in `src/client/`
6. **Test manually** - Run server + client together

### Step 4: Testing

```bash
# Unit tests
ctest --preset debug

# Manual testing (two terminals)
./build/debug/city_server    # Terminal 1
./build/debug/city_client    # Terminal 2
```

## Known Issues / TODOs

### High Priority
- [ ] Vulkan rendering is completely stubbed
- [ ] Client-server handshake needs testing
- [ ] Content system not wired up

### Medium Priority
- [ ] No error handling on network disconnect
- [ ] No graceful shutdown
- [ ] World.each() could be optimized

### Low Priority
- [ ] No logging system
- [ ] No configuration files
- [ ] No admin commands

## Code Style

- **Naming**: `snake_case` for functions/variables, `PascalCase` for types
- **Headers**: `.hpp` for C++ headers
- **Includes**: Prefer `"relative/path.hpp"` for project files
- **Memory**: Use `std::unique_ptr` for ownership, raw pointers for non-owning
- **Errors**: Throw exceptions for unrecoverable, return optional/bool for recoverable

## Quick Reference

### Adding a New Component

```cpp
// 1. Create header in src/core/game/components/
struct MyComponent {
    int value;

    void serialize(Serializer& s) const { s.write_i32(value); }
    void deserialize(Deserializer& d) { value = d.read_i32(); }
};

// 2. Use it
world.add_component<MyComponent>(entity, {42});
auto* comp = world.get_component<MyComponent>(entity);
```

### Adding a New System

```cpp
// 1. Create in src/core/game/systems/ (or client/server specific)
class MySystem : public System {
public:
    void update(World& world, f32 dt) override {
        world.each<MyComponent>([dt](Entity e, MyComponent& c) {
            // Update logic
        });
    }
};

// 2. Add to world
world.add_system<MySystem>();
```

### Adding a New Message Type

```cpp
// 1. Add to protocol.hpp
enum class MessageType : u8 {
    // ...existing...
    MyMessage = 0x60,
};

// 2. Create payload struct in message.hpp
struct MyMessagePayload {
    u32 data;
    void serialize(Serializer& s) const { s.write_u32(data); }
    void deserialize(Deserializer& d) { data = d.read_u32(); }
};

// 3. Send it
session.send(Message::create(MessageType::MyMessage, MyMessagePayload{42}));

// 4. Handle it
case MessageType::MyMessage: {
    MyMessagePayload payload;
    payload.deserialize(msg.reader());
    // Process...
    break;
}
```
