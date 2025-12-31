#pragma once

#include "core/util/types.hpp"

namespace city::net {

// Protocol version for compatibility checking
constexpr u32 PROTOCOL_VERSION = 1;

// Tick rate: 60 ticks/second (~16.67ms per tick)
constexpr f32 TICK_RATE = 60.0f;
constexpr f32 TICK_INTERVAL = 1.0f / TICK_RATE;
constexpr u32 TICK_INTERVAL_MS = 16; // Rounded for timer precision

// Network constants
constexpr u16 DEFAULT_PORT = 7777;
constexpr u32 MAX_PLAYERS = 100;
constexpr u32 MAX_PACKET_SIZE = 1400; // Safe MTU size
constexpr u32 MAX_MESSAGE_SIZE = 65536; // For fragmented messages

// Content transfer
constexpr u32 CONTENT_CHUNK_SIZE = 65536; // 64KB chunks

// Message types
enum class MessageType : u8 {
    // Connection (0x00 - 0x0F)
    ClientHello        = 0x01,
    ServerHello        = 0x02,
    Disconnect         = 0x03,
    Ping               = 0x04,
    Pong               = 0x05,
    Kick               = 0x06,

    // Content transfer (0x10 - 0x1F)
    ContentRequest     = 0x10,
    ContentManifest    = 0x11,
    ContentChunk       = 0x12,
    ContentComplete    = 0x13,

    // Game state (0x20 - 0x2F)
    FullState          = 0x20,
    DeltaState         = 0x21,
    EntitySpawn        = 0x22,
    EntityDespawn      = 0x23,
    EntityUpdate       = 0x24,
    ChunkData          = 0x25,

    // Player input (0x30 - 0x3F)
    PlayerInput        = 0x30,
    InputAck           = 0x31,

    // Chat (0x40 - 0x4F)
    ChatMessage        = 0x40,
    ChatBroadcast      = 0x41,

    // Round management (0x50 - 0x5F)
    RoundStart         = 0x50,
    RoundEnd           = 0x51,
    RoundStatus        = 0x52,

    // Admin (0xF0 - 0xFF)
    RconCommand        = 0xF0,
    RconResponse       = 0xF1,
};

// Reliability modes (for ENet channel mapping)
enum class Reliability : u8 {
    Unreliable = 0,        // Fire and forget (movement updates)
    UnreliableSequenced,   // Drop old packets
    Reliable,              // Guaranteed delivery (chat, spawns)
    ReliableOrdered,       // Guaranteed + ordered (state sync)
};

// Disconnect reasons
enum class DisconnectReason : u8 {
    Unknown = 0,
    ClientQuit,
    ServerShutdown,
    Timeout,
    Kicked,
    Banned,
    VersionMismatch,
    AuthFailed,
    ServerFull,
};

// Chat channels
enum class ChatChannel : u8 {
    Global = 0,     // All players
    Local,          // Nearby players only
    Team,           // Same team/faction
    Whisper,        // Direct message
    System = 255,   // Server announcements
};

// Helper to get reliability for message type
constexpr Reliability get_reliability(MessageType type) {
    switch (type) {
        case MessageType::PlayerInput:
        case MessageType::EntityUpdate:
        case MessageType::DeltaState:
            return Reliability::UnreliableSequenced;

        case MessageType::Ping:
        case MessageType::Pong:
            return Reliability::Unreliable;

        case MessageType::ClientHello:
        case MessageType::ServerHello:
        case MessageType::EntitySpawn:
        case MessageType::EntityDespawn:
        case MessageType::ChatMessage:
        case MessageType::ChatBroadcast:
        case MessageType::ContentChunk:
        case MessageType::RoundStart:
        case MessageType::RoundEnd:
            return Reliability::Reliable;

        default:
            return Reliability::ReliableOrdered;
    }
}

} // namespace city::net
