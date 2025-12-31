# City - Networking Protocol

## Overview

City uses a custom binary protocol over ENet (reliable UDP) for all client-server communication. This document details the protocol format, message types, and synchronization strategies.

## Protocol Constants

```cpp
// Protocol version (increment on breaking changes)
constexpr u32 PROTOCOL_VERSION = 1;

// Timing
constexpr f32 TICK_RATE = 60.0f;        // Ticks per second
constexpr f32 TICK_INTERVAL = 1.0f / 60; // ~16.67ms

// Network
constexpr u16 DEFAULT_PORT = 7777;
constexpr u32 MAX_PLAYERS = 100;
constexpr u32 MAX_PACKET_SIZE = 1400;   // Safe MTU
```

## Message Format

Every network message follows this structure:

```
┌──────────────────────────────────────────────────────────────┐
│                        MESSAGE                               │
├─────────┬───────────┬─────────────┬─────────────────────────┤
│ Type    │ Sequence  │ Length      │ Payload                 │
│ 1 byte  │ 2 bytes   │ 2 bytes     │ N bytes                 │
└─────────┴───────────┴─────────────┴─────────────────────────┘
```

| Field | Size | Description |
|-------|------|-------------|
| Type | 1 byte | MessageType enum value |
| Sequence | 2 bytes | Packet sequence number |
| Length | 2 bytes | Payload length in bytes |
| Payload | Variable | Serialized message data |

**Total header size: 5 bytes**

## Serialization

### Primitive Types

| Type | Size | Format |
|------|------|--------|
| u8/i8 | 1 byte | Raw byte |
| u16/i16 | 2 bytes | Big-endian |
| u32/i32 | 4 bytes | Big-endian |
| u64/i64 | 8 bytes | Big-endian |
| f32 | 4 bytes | IEEE 754 (via u32) |
| f64 | 8 bytes | IEEE 754 (via u64) |
| bool | 1 byte | 0 or 1 |

### Compound Types

| Type | Format |
|------|--------|
| string | varint length + UTF-8 bytes |
| Vec2f | f32 x + f32 y |
| Vec2i | i32 x + i32 y |
| bytes | length + raw bytes |

### Variable-Length Integer (varint)

For values that are usually small:

```
Value < 128:      1 byte  [0xxxxxxx]
Value < 16384:    2 bytes [1xxxxxxx 0xxxxxxx]
...and so on, 7 bits per byte, high bit indicates continuation
```

## Message Types

### Connection Messages (0x01-0x0F)

#### ClientHello (0x01)
Client → Server: Initial connection request

```
┌──────────────────┬─────────────────┬────────────────────┐
│ protocol_version │ client_version  │ player_name        │
│ u32              │ string          │ string             │
└──────────────────┴─────────────────┴────────────────────┘
```

#### ServerHello (0x02)
Server → Client: Connection accepted

```
┌──────────────────┬────────────┬─────────────┬────────────┬─────────────────┐
│ protocol_version │ server_id  │ server_name │ session_id │ player_entity   │
│ u32              │ string     │ string      │ u32        │ NetEntityId     │
└──────────────────┴────────────┴─────────────┴────────────┴─────────────────┘
```

#### Disconnect (0x03)
Either → Either: Connection ending

```
┌────────────┬───────────┐
│ reason     │ message   │
│ u8         │ string    │
└────────────┴───────────┘

Reason values:
0 = Unknown
1 = ClientQuit
2 = ServerShutdown
3 = Timeout
4 = Kicked
5 = Banned
6 = VersionMismatch
7 = AuthFailed
8 = ServerFull
```

#### Ping (0x04) / Pong (0x05)
Latency measurement (empty payload)

### Content Transfer (0x10-0x1F)

#### ContentRequest (0x10)
Client → Server: Request missing assets

```
┌───────────────┬──────────────────────────┐
│ count         │ resource_ids[]           │
│ u32           │ u64 × count              │
└───────────────┴──────────────────────────┘
```

#### ContentManifest (0x11)
Server → Client: Asset list

```
┌────────────┬─────────────┬───────────┬────────────────┬─────────────┐
│ server_id  │ server_name │ version   │ total_size     │ assets[]    │
│ string     │ string      │ u32       │ u64            │ AssetEntry× │
└────────────┴─────────────┴───────────┴────────────────┴─────────────┘

AssetEntry:
┌────────────┬──────────┬──────────┬────────────┬──────────────┐
│ resource_id│ type     │ path     │ size       │ checksum     │
│ u64        │ u8       │ string   │ u64        │ u64          │
└────────────┴──────────┴──────────┴────────────┴──────────────┘
```

#### ContentChunk (0x12)
Server → Client: Asset data chunk

```
┌─────────────┬─────────────┬──────────────┬─────────────┐
│ resource_id │ chunk_index │ total_chunks │ data        │
│ u64         │ u32         │ u32          │ bytes       │
└─────────────┴─────────────┴──────────────┴─────────────┘
```

#### ContentComplete (0x13)
Server → Client: All content sent (empty payload)

### Game State (0x20-0x2F)

#### FullState (0x20)
Server → Client: Complete world state (on connect)

```
┌─────────────┬─────────────────┬─────────────────────────┐
│ tick        │ tilemap         │ entities[]              │
│ u32         │ TileMap data    │ EntityData×             │
└─────────────┴─────────────────┴─────────────────────────┘

EntityData:
┌──────────────┬────────────────┬─────────────────────────┐
│ net_entity_id│ component_flags│ components[]            │
│ u32          │ u8             │ ComponentData×          │
└──────────────┴────────────────┴─────────────────────────┘
```

#### DeltaState (0x21)
Server → Clients: Per-tick state update

```
┌─────────────┬───────────────────┬─────────────────────────┐
│ tick        │ last_acked_input  │ entity_updates[]        │
│ u32         │ u32               │ EntityUpdate×           │
└─────────────┴───────────────────┴─────────────────────────┘

EntityUpdate:
┌──────────────┬────────────────┬────────────────┐
│ net_entity_id│ position       │ velocity       │
│ u32          │ Vec2f          │ Vec2f          │
└──────────────┴────────────────┴────────────────┘
```

#### EntitySpawn (0x22)
Server → Clients: New entity created

```
┌──────────────┬────────────────┬─────────────────────────┐
│ net_entity_id│ component_flags│ components[]            │
│ u32          │ u8             │ ComponentData×          │
└──────────────┴────────────────┴─────────────────────────┘
```

#### EntityDespawn (0x23)
Server → Clients: Entity removed

```
┌──────────────┐
│ net_entity_id│
│ u32          │
└──────────────┘
```

### Player Input (0x30-0x3F)

#### PlayerInput (0x30)
Client → Server: Player input for a tick

```
┌──────────┬───────────────────┬──────────┬──────────┬───────────┬─────────────┐
│ tick     │ last_received_tick│ move_x   │ move_y   │ buttons   │ target_tile │
│ u32      │ u32               │ i8       │ i8       │ u8        │ Vec2i       │
└──────────┴───────────────────┴──────────┴──────────┴───────────┴─────────────┘

move_x, move_y: -1, 0, or 1
buttons: bit flags (0x01 = interact, 0x02 = secondary, etc.)
```

#### InputAck (0x31)
Server → Client: Input acknowledged

```
┌──────────────────┐
│ tick             │
│ u32              │
└──────────────────┘
```

### Chat (0x40-0x4F)

#### ChatMessage (0x40)
Client → Server: Send chat message

```
┌───────────┬────────────┬────────────┬─────────────┐
│ channel   │ target     │ content    │ (unused)    │
│ u8        │ string     │ string     │             │
└───────────┴────────────┴────────────┴─────────────┘

Channel values:
0 = Global
1 = Local (nearby)
2 = Team
3 = Whisper
255 = System
```

#### ChatBroadcast (0x41)
Server → Clients: Broadcast chat message

```
┌───────────┬────────────┬────────────┬─────────────┐
│ channel   │ sender     │ target     │ content     │
│ u8        │ string     │ string     │ string      │
└───────────┴────────────┴────────────┴─────────────┘
```

## Reliability Modes

ENet provides multiple reliability modes. City uses 2 channels:

| Channel | Mode | Use Cases |
|---------|------|-----------|
| 0 | Reliable Ordered | State sync, spawns, chat |
| 1 | Unreliable Sequenced | Position updates, input |

### Message Reliability Mapping

| Message Type | Reliability | Channel |
|--------------|-------------|---------|
| ClientHello | Reliable | 0 |
| ServerHello | Reliable | 0 |
| Disconnect | Reliable | 0 |
| Ping/Pong | Unreliable | 1 |
| ContentChunk | Reliable | 0 |
| FullState | Reliable Ordered | 0 |
| DeltaState | Unreliable Sequenced | 1 |
| EntitySpawn | Reliable | 0 |
| EntityDespawn | Reliable | 0 |
| PlayerInput | Unreliable Sequenced | 1 |
| ChatMessage | Reliable | 0 |

## Connection Flow

### Client Connection Sequence

```
Client                                    Server
  │                                          │
  │──────── [Connect] ──────────────────────>│
  │                                          │ Accept connection
  │                                          │ Create ClientSession
  │<─────── [Connected event] ───────────────│
  │                                          │
  │──────── ClientHello ────────────────────>│
  │         - protocol_version               │ Validate version
  │         - client_version                 │ Create player entity
  │         - player_name                    │
  │                                          │
  │<─────── ServerHello ─────────────────────│
  │         - protocol_version               │
  │         - server_id                      │
  │         - session_id                     │
  │         - player_entity_id               │
  │                                          │
  │<─────── ContentManifest ─────────────────│
  │         - asset list                     │
  │                                          │
  │──────── ContentRequest ─────────────────>│ (if missing assets)
  │         - missing asset IDs              │
  │                                          │
  │<─────── ContentChunk ────────────────────│ (repeated)
  │         - asset data                     │
  │                                          │
  │<─────── ContentComplete ─────────────────│
  │                                          │
  │<─────── FullState ───────────────────────│
  │         - tilemap                        │
  │         - all entities                   │
  │                                          │
  │ ═══════════════════════════════════════ │
  │              GAME LOOP                   │
  │ ═══════════════════════════════════════ │
```

### Game Loop Synchronization

```
Client (60 Hz)                           Server (60 Hz)
  │                                          │
  ├─ Capture input for tick N                │
  ├─ Apply input locally (predict)           │
  │                                          │
  │──────── PlayerInput (tick N) ───────────>│
  │                                          ├─ Queue input
  │                                          │
  │                                          ├─ Tick N:
  │                                          │  ├─ Process queued inputs
  │                                          │  ├─ Run game systems
  │                                          │  └─ Generate delta
  │                                          │
  │<─────── DeltaState (tick N) ─────────────│
  │         - last_acked_input = N           │
  │         - entity positions               │
  │                                          │
  ├─ Compare prediction vs server            │
  ├─ If mismatch: reconcile                  │
  │  ├─ Snap to server state                 │
  │  └─ Replay inputs N+1...current          │
  ├─ Acknowledge input N                     │
  │                                          │
```

## Delta Compression

### What Gets Synced

Every tick, the server broadcasts:
- Current tick number
- Last acknowledged input (per client)
- Changed entity positions/velocities

### Interest Management

For scalability (50+ players):

1. **Proximity-based**: Only sync entities within view distance
2. **Priority-based**: Nearby entities sync more frequently
3. **Delta only**: Only sync changed data

```
View Distance Tiers:
┌─────────────────────────────────────────┐
│                 FULL SYNC               │  0-10 tiles: every tick
│     ┌─────────────────────────┐         │
│     │      REDUCED SYNC       │         │  10-30 tiles: every 3 ticks
│     │  ┌───────────────────┐  │         │
│     │  │   OUT OF VIEW     │  │         │  30+ tiles: not synced
│     │  │   (not synced)    │  │         │
│     │  └───────────────────┘  │         │
│     └─────────────────────────┘         │
└─────────────────────────────────────────┘
```

## Client-Side Prediction

### Input Buffer

Client maintains a buffer of unacknowledged inputs:

```cpp
struct InputBuffer {
    array<InputSnapshot, 128> buffer;  // Circular buffer
    u32 last_acked_tick;               // Last confirmed by server
    u32 latest_tick;                   // Most recent input

    void add(InputSnapshot input);
    void acknowledge(u32 tick);
    vector<InputSnapshot> get_unacked();
};
```

### Prediction Loop

```cpp
void PredictionSystem::record_input(InputSnapshot input) {
    // 1. Store input
    input_buffer.add(input);

    // 2. Apply locally
    Entity player = world.get_by_net_id(local_player_id);
    apply_input(player, input);
}

void PredictionSystem::on_server_state(u32 tick, Vec2f position) {
    // 1. Get predicted position for this tick
    Vec2f predicted = state_buffer.get(tick);

    // 2. Compare with server
    if (distance(predicted, position) > THRESHOLD) {
        // 3. Snap to server
        transform.position = position;

        // 4. Replay unacked inputs
        for (auto& input : input_buffer.get_unacked()) {
            apply_input(player, input);
        }
    }

    // 5. Acknowledge processed inputs
    input_buffer.acknowledge(tick);
}
```

### Interpolation

Remote entities are interpolated between known states:

```cpp
// Buffer last few received positions
struct InterpolationBuffer {
    struct Sample { u32 tick; Vec2f position; };
    array<Sample, 10> samples;

    Vec2f interpolate(f32 render_time) {
        // Find two samples bracketing render_time
        // Linearly interpolate between them
    }
};

// Render 100ms behind server
constexpr f32 INTERP_DELAY = 0.1f;
Vec2f render_pos = interp_buffer.interpolate(server_time - INTERP_DELAY);
```

## Error Handling

### Version Mismatch

```
Client                    Server
  │                          │
  │── ClientHello ──────────>│
  │   version: 2             │ Server version: 1
  │                          │
  │<── Disconnect ───────────│
  │    reason: VersionMismatch
  │    message: "Server requires protocol v1"
```

### Timeout

```
- Client: No server response for 5 seconds
- Server: No client message for 30 seconds

Result: Disconnect(Timeout)
```

### Rate Limiting

```cpp
constexpr u32 MAX_INPUTS_PER_SECOND = 100;
constexpr u32 MAX_CHAT_PER_MINUTE = 30;

// Server validates:
if (session.inputs_this_second > MAX_INPUTS_PER_SECOND) {
    // Ignore excess inputs
}
```

## Security Considerations

### Server Authority

- Server validates all movement (no teleporting)
- Server validates all actions (no impossible actions)
- Server controls spawn positions
- Server controls inventory changes

### Validation Examples

```cpp
// Movement validation
Vec2f requested = input.position;
Vec2f current = transform.position;
f32 distance = current.distance(requested);
f32 max_distance = player.speed * TICK_INTERVAL * 1.5f; // 50% tolerance

if (distance > max_distance) {
    // Reject: moving too fast
    return;
}

// Action validation
if (input.interact && !is_adjacent(player, target)) {
    // Reject: too far to interact
    return;
}
```

### Anti-Cheat

- Position sanity checks
- Action rate limiting
- Logging suspicious activity
- Admin notification system

## Debugging

### Logging

```cpp
// Enable network logging
#define NET_LOG_LEVEL 2

// Levels:
// 0 = Errors only
// 1 = + Warnings
// 2 = + Info (connections, important events)
// 3 = + Debug (all messages)
// 4 = + Trace (per-tick data)
```

### Network Stats

Track and display:
- Ping (RTT)
- Packet loss %
- Bandwidth (bytes/sec)
- Tick delta (ms behind server)

```cpp
struct NetworkStats {
    u32 ping_ms;
    f32 packet_loss;
    u32 bytes_sent_per_sec;
    u32 bytes_recv_per_sec;
    i32 tick_offset;
};
```
