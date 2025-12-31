# City - Development Progress

This document tracks implementation progress and provides guidance for continuing development.

## Current Status: Phase 1 Complete (Foundation)

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

### Phase 2: Networking Core 🔲 NOT STARTED

Make client and server actually communicate.

| Task | Status | Priority | Description |
|------|--------|----------|-------------|
| Server startup | 🔲 | HIGH | ENet host initialization, listening |
| Client connect | 🔲 | HIGH | Connect to server, handshake |
| Hello exchange | 🔲 | HIGH | Protocol version validation |
| Session management | 🔲 | HIGH | Track connected clients |
| Player spawn | 🔲 | HIGH | Create entity on connect |
| Input transmission | 🔲 | HIGH | Send PlayerInput messages |
| State broadcast | 🔲 | HIGH | Send DeltaState to clients |
| Disconnect handling | 🔲 | MEDIUM | Clean up on disconnect |
| Ping/pong | 🔲 | LOW | Latency measurement |

**Key Files to Modify:**
- `src/server/net/server_connection.cpp` - Connection handling
- `src/client/net/client_connection.cpp` - Connect logic
- `src/server/server.cpp` - Message dispatch
- `src/client/client.cpp` - Message handling

**Testing:**
```bash
# Terminal 1: Start server
./build/debug/city_server

# Terminal 2: Start client
./build/debug/city_client
```

### Phase 3: Rendering 🔲 NOT STARTED

Implement actual Vulkan rendering.

| Task | Status | Priority | Description |
|------|--------|----------|-------------|
| Vulkan instance | 🔲 | HIGH | Create VkInstance |
| Surface creation | 🔲 | HIGH | SDL3 Vulkan surface |
| Device selection | 🔲 | HIGH | Pick physical/logical device |
| Swapchain | 🔲 | HIGH | Create swapchain |
| Render pass | 🔲 | HIGH | Basic render pass |
| Pipeline | 🔲 | HIGH | 2D sprite pipeline |
| Sprite batching | 🔲 | MEDIUM | Efficient sprite rendering |
| Tile rendering | 🔲 | MEDIUM | Render tile chunks |
| Camera | 🔲 | MEDIUM | View transformation |
| Texture loading | 🔲 | MEDIUM | Load sprites from files |

**Key Files to Modify:**
- `src/client/render/vulkan/vk_context.cpp` - Vulkan setup
- `src/client/render/renderer.cpp` - Render loop
- `src/client/render/sprite_batch.cpp` - Batched drawing

**Resources:**
- [Vulkan Tutorial](https://vulkan-tutorial.com/)
- [SDL3 Vulkan Guide](https://wiki.libsdl.org/SDL3/CategoryVulkan)

### Phase 4: Player Movement 🔲 NOT STARTED

Complete movement with prediction.

| Task | Status | Priority | Description |
|------|--------|----------|-------------|
| Server movement | 🔲 | HIGH | Authoritative position update |
| Collision detection | 🔲 | HIGH | Tile-based collision |
| Input processing | 🔲 | HIGH | Server processes client input |
| State sync | 🔲 | HIGH | Send positions to clients |
| Client prediction | 🔲 | HIGH | Predict local movement |
| Reconciliation | 🔲 | HIGH | Correct prediction errors |
| Interpolation | 🔲 | MEDIUM | Smooth remote players |

**Key Files:**
- `src/core/game/systems/movement.cpp` - Movement logic
- `src/server/systems/input_processor.cpp` - Input handling
- `src/client/prediction/prediction.cpp` - Prediction/reconciliation

### Phase 5: Entity Sync 🔲 NOT STARTED

Full entity synchronization.

| Task | Status | Priority | Description |
|------|--------|----------|-------------|
| Full state send | 🔲 | HIGH | Send all entities on connect |
| Delta tracking | 🔲 | HIGH | Track changed components |
| Delta broadcast | 🔲 | HIGH | Send only changes per tick |
| Entity spawn | 🔲 | MEDIUM | Notify clients of new entities |
| Entity despawn | 🔲 | MEDIUM | Notify clients of removed entities |
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
