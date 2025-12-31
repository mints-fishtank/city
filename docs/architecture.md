# City - Technical Architecture

## Overview

City is a top-down grid-based multiplayer roleplay game built with a client-server architecture. The codebase is structured to allow servers to fork and customize content while clients dynamically download server-specific assets.

## Technology Stack

| Component | Technology | Rationale |
|-----------|------------|-----------|
| Language | C++20 | Performance, modern features, cross-platform |
| Build | CMake 3.21+ | Industry standard, FetchContent for deps |
| Compiler | Clang/LLVM | Consistent cross-platform behavior |
| Rendering | SDL3 + Vulkan | Modern graphics API, cross-platform |
| Networking | ENet | Reliable UDP, proven in games, lightweight |
| Serialization | Custom binary | Compact, fast, full control |
| JSON | nlohmann/json | Content definitions, config files |
| Compression | zstd | Fast compression for content transfer |
| Testing | GoogleTest | Standard C++ testing framework |

## Project Structure

```
city/
├── src/
│   ├── core/                  # Shared library (client + server)
│   │   ├── ecs/               # Entity Component System
│   │   ├── grid/              # Tile-based world
│   │   ├── net/               # Networking primitives
│   │   ├── content/           # Asset loading
│   │   ├── util/              # Math, types, utilities
│   │   └── game/              # Shared game logic
│   │       ├── components/    # Transform, Player, Sprite, etc.
│   │       └── systems/       # Movement, Collision
│   │
│   ├── client/                # Game client executable
│   │   ├── render/            # SDL3 + Vulkan rendering
│   │   │   └── vulkan/        # Vulkan context, pipelines
│   │   ├── input/             # Input handling, keybinds
│   │   ├── prediction/        # Client-side prediction
│   │   ├── net/               # Server connection
│   │   └── ui/                # Chat, HUD
│   │
│   └── server/                # Dedicated server executable
│       ├── simulation/        # Game state, rounds
│       ├── net/               # Client management
│       └── systems/           # Server-only systems
│
├── content/                   # Base game content
├── tests/                     # Unit tests
├── third_party/               # Dependencies (via FetchContent)
├── tools/                     # Asset packer, etc.
└── docs/                      # Documentation
```

## Core Systems

### Entity Component System (ECS)

Location: `src/core/ecs/`

A simple, debuggable ECS using sparse-set component storage:

```cpp
// Entity: 32-bit index + 32-bit generation
struct Entity {
    u32 index;
    u32 generation;
};

// World manages entities and components
World world;
Entity player = world.create();
world.add_component<Transform>(player, {.position = {10, 20}});
world.add_component<Player>(player, {.name = "Alice"});

// Iterate entities with specific components
world.each<Transform, Player>([](Entity e, Transform& t, Player& p) {
    // Process entities with both components
});
```

**Key Features:**
- O(1) component add/remove/lookup via sparse sets
- Generation-based entity recycling (safe handles)
- Network entity ID mapping for multiplayer sync
- Type-erased component pools (no registration macros)

### Grid System

Location: `src/core/grid/`

Chunk-based tile storage for efficient rendering and serialization:

```
World Space          Chunk (16x16)
┌──────────────┐    ┌────────────────┐
│ Chunk │ Chunk│    │ Tile Tile Tile │
│ (0,0) │(16,0)│    │ Tile Tile Tile │
├───────┼──────┤    │ Tile Tile Tile │
│ Chunk │ Chunk│    └────────────────┘
│(0,16) │(16,16)
└───────┴──────┘
```

**Tile Structure:**
```cpp
struct Tile {
    u16 floor_id;    // Floor sprite
    u16 wall_id;     // Wall sprite (0 = none)
    u16 overlay_id;  // Decorations
    TileFlags flags; // Solid, Opaque, etc.
};
```

**Key Features:**
- 16x16 tile chunks for cache efficiency
- Negative coordinate support (infinite world)
- Line-of-sight calculation (Bresenham)
- Pathfinding helpers (passable neighbors)

### Networking

Location: `src/core/net/`

Custom binary protocol over ENet reliable UDP:

```
Message Format:
┌─────────┬──────────┬────────────┬─────────────┐
│ Type(1) │ Seq(2)   │ Length(2)  │ Payload(N)  │
└─────────┴──────────┴────────────┴─────────────┘
```

**Serialization:**
```cpp
Serializer s;
s.write_u32(player_id);
s.write_string(name);
s.write_vec2f(position);

Deserializer d(s.data());
u32 id = d.read_u32();
std::string name = d.read_string();
Vec2f pos = d.read_vec2f();
```

**Reliability Modes:**
- `Unreliable`: Fire and forget (ping/pong)
- `UnreliableSequenced`: Drop old packets (position updates)
- `Reliable`: Guaranteed delivery (chat, spawns)
- `ReliableOrdered`: Guaranteed + ordered (state sync)

### Content System

Location: `src/core/content/`

Manifest-based asset management for server customization:

```cpp
// Server generates manifest from content directory
ContentManifest manifest = ContentManifest::from_directory("content", "my-server");

// Client compares manifests, downloads missing assets
auto missing = loader.get_missing_assets();
for (auto id : missing) {
    request_asset(id);
}
```

## Client Architecture

### Main Loop

```cpp
while (running) {
    handle_events();           // SDL events

    // Fixed timestep (60 Hz)
    accumulator += dt;
    while (accumulator >= TICK_INTERVAL) {
        process_input();       // Capture + send input
        predict_locally();     // Client-side prediction
        update_systems();      // Run game systems
        accumulator -= TICK_INTERVAL;
    }

    process_network();         // Receive server updates
    reconcile();               // Fix prediction errors
    render(interpolation);     // Interpolate between states
}
```

### Client-Side Prediction

The client predicts local player movement for responsive feel:

1. **Input Capture**: Record input with tick number
2. **Local Prediction**: Apply input immediately
3. **Send to Server**: Transmit input over network
4. **Server Processing**: Server applies input authoritatively
5. **State Received**: Client receives server state for tick N
6. **Reconciliation**: If mismatch, snap to server + replay inputs N+1...current

```cpp
class PredictionSystem {
    InputBuffer input_buffer_;      // Unacked inputs

    void on_server_state(u32 tick, Vec2f position) {
        // Compare prediction with server
        if (distance(predicted, position) > threshold) {
            // Snap to server position
            transform.position = position;
            // Replay unacked inputs
            for (auto& input : input_buffer_.get_unacked()) {
                apply_input(input);
            }
        }
        input_buffer_.acknowledge(tick);
    }
};
```

## Server Architecture

### Main Loop

```cpp
while (running) {
    process_network();          // Accept connections, receive inputs

    // Fixed timestep (60 Hz)
    accumulator += dt;
    while (accumulator >= TICK_INTERVAL) {
        process_queued_inputs(); // Apply client inputs
        update_systems();        // Authoritative simulation
        broadcast_state();       // Send delta updates
        ++current_tick;
        accumulator -= TICK_INTERVAL;
    }
}
```

### Client Session Lifecycle

```
Connect → Hello → Content Download → Full State → Playing → Disconnect
   │         │            │              │           │          │
   └─ Assign ─┴─ Validate ─┴─── Stream ───┴─ Spawn ───┴─ Sync ───┴─ Cleanup
      session    version      assets       player     delta      entity
```

### Round Management

```
Lobby → Starting → Playing → Ending → Lobby
  │        │          │         │        │
  └─Wait───┴─Countdown─┴─3 hours─┴─Results─┘
    players   10 sec              30 sec
```

## Data Flow

### Player Movement

```
Client                          Server
  │                               │
  │ Capture Input (tick 100)      │
  │ Apply locally (prediction)    │
  │──── PlayerInput ─────────────>│
  │                               │ Queue input
  │                               │ Process at tick 100
  │                               │ Update position authoritatively
  │<──── DeltaState (tick 100) ───│
  │ Compare with prediction       │
  │ Reconcile if needed           │
  │                               │
```

### Entity Synchronization

```
Server                          All Clients
  │                               │
  │ Entity spawned                │
  │──── EntitySpawn ─────────────>│ Create local entity
  │                               │
  │ Each tick:                    │
  │──── DeltaState ──────────────>│ Update positions
  │     (changed entities only)   │
  │                               │
  │ Entity destroyed              │
  │──── EntityDespawn ───────────>│ Remove local entity
  │                               │
```

## Build Configuration

### CMake Presets

| Preset | Client | Server | Tests | Use Case |
|--------|--------|--------|-------|----------|
| `debug` | ✓ | ✓ | ✓ | Development |
| `release` | ✓ | ✓ | ✗ | Distribution |
| `client-debug` | ✓ | ✗ | ✗ | Client dev only |
| `server-debug` | ✗ | ✓ | ✗ | Server dev only |

### Build Commands

```bash
# Full development build
cmake --preset debug
cmake --build --preset debug

# Server only (headless)
cmake --preset server-release
cmake --build --preset server-release
```

## Dependencies

All dependencies are fetched automatically via CMake FetchContent:

| Library | Version | Purpose |
|---------|---------|---------|
| SDL3 | main | Window, input, Vulkan surface |
| ENet | 1.3.18 | Reliable UDP networking |
| nlohmann_json | 3.11.3 | JSON parsing |
| zstd | 1.5.5 | Compression |
| stb | latest | Image loading (header-only) |
| GoogleTest | 1.14.0 | Unit testing |

## Performance Targets

| Metric | Target | Notes |
|--------|--------|-------|
| Tick Rate | 60 Hz | ~16.67ms per tick |
| Max Players | 100 | Per server instance |
| Round Duration | ~3 hours | Configurable |
| Network Update | Every tick | Delta compression |
| Content Download | On connect | Cached locally |
