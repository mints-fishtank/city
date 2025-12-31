#pragma once

#include "protocol.hpp"
#include "serialization.hpp"
#include "core/ecs/entity.hpp"
#include <vector>
#include <optional>
#include <memory>

namespace city::net {

// Message header (5 bytes)
struct MessageHeader {
    MessageType type;
    u16 sequence;        // For sequenced/ordered messages
    u16 payload_length;

    static constexpr size_t SIZE = 5;

    void serialize(Serializer& s) const {
        s.write_u8(static_cast<u8>(type));
        s.write_u16(sequence);
        s.write_u16(payload_length);
    }

    void deserialize(Deserializer& d) {
        type = static_cast<MessageType>(d.read_u8());
        sequence = d.read_u16();
        payload_length = d.read_u16();
    }
};

// Complete network message
class Message {
public:
    Message() = default;
    Message(MessageType type, std::vector<u8> payload, u16 sequence = 0)
        : type_(type)
        , sequence_(sequence)
        , payload_(std::move(payload)) {}

    // Accessors
    MessageType type() const { return type_; }
    u16 sequence() const { return sequence_; }
    std::span<const u8> payload() const { return payload_; }
    size_t payload_size() const { return payload_.size(); }

    // Get a deserializer for the payload
    Deserializer reader() const { return Deserializer{payload_}; }

    // Create a message with serialized payload
    template<typename T>
    static Message create(MessageType type, const T& data, u16 sequence = 0) {
        Serializer s;
        data.serialize(s);
        return Message{type, s.take(), sequence};
    }

    // Create an empty message (header only)
    static Message create_empty(MessageType type, u16 sequence = 0) {
        return Message{type, {}, sequence};
    }

    // Encode message to bytes (header + payload)
    std::vector<u8> encode() const {
        Serializer s;
        MessageHeader header{
            type_,
            sequence_,
            static_cast<u16>(payload_.size())
        };
        header.serialize(s);
        s.write_bytes(payload_);
        return s.take();
    }

    // Parse message from bytes
    static std::optional<Message> parse(std::span<const u8> data) {
        if (data.size() < MessageHeader::SIZE) {
            return std::nullopt;
        }

        Deserializer d{data};
        MessageHeader header;
        header.deserialize(d);

        if (d.remaining() < header.payload_length) {
            return std::nullopt;
        }

        std::vector<u8> payload = d.read_bytes(header.payload_length);
        return Message{header.type, std::move(payload), header.sequence};
    }

    // Check if we have enough data for a complete message
    static std::optional<size_t> peek_size(std::span<const u8> data) {
        if (data.size() < MessageHeader::SIZE) {
            return std::nullopt;
        }

        Deserializer d{data};
        d.skip(1);  // type
        d.skip(2);  // sequence
        u16 payload_length = d.read_u16();

        return MessageHeader::SIZE + payload_length;
    }

private:
    MessageType type_{};
    u16 sequence_{0};
    std::vector<u8> payload_;
};

// ========== Common Message Payloads ==========

// Client -> Server: Initial connection
struct ClientHelloPayload {
    u32 protocol_version;
    std::string client_version;
    std::string player_name;

    void serialize(Serializer& s) const {
        s.write_u32(protocol_version);
        s.write_string(client_version);
        s.write_string(player_name);
    }

    void deserialize(Deserializer& d) {
        protocol_version = d.read_u32();
        client_version = d.read_string();
        player_name = d.read_string();
    }
};

// Server -> Client: Connection accepted
struct ServerHelloPayload {
    u32 protocol_version;
    std::string server_id;
    std::string server_name;
    u32 session_id;           // Assigned session ID for this client
    NetEntityId player_entity_id;  // Network ID of the client's player entity

    void serialize(Serializer& s) const {
        s.write_u32(protocol_version);
        s.write_string(server_id);
        s.write_string(server_name);
        s.write_u32(session_id);
        s.write_u32(player_entity_id);
    }

    void deserialize(Deserializer& d) {
        protocol_version = d.read_u32();
        server_id = d.read_string();
        server_name = d.read_string();
        session_id = d.read_u32();
        player_entity_id = d.read_u32();
    }
};

// Disconnect notification
struct DisconnectPayload {
    DisconnectReason reason;
    std::string message;

    void serialize(Serializer& s) const {
        s.write_u8(static_cast<u8>(reason));
        s.write_string(message);
    }

    void deserialize(Deserializer& d) {
        reason = static_cast<DisconnectReason>(d.read_u8());
        message = d.read_string();
    }
};

// Chat message
struct ChatPayload {
    ChatChannel channel;
    std::string sender;      // Empty for system messages
    std::string target;      // For whispers
    std::string content;

    void serialize(Serializer& s) const {
        s.write_u8(static_cast<u8>(channel));
        s.write_string(sender);
        s.write_string(target);
        s.write_string(content);
    }

    void deserialize(Deserializer& d) {
        channel = static_cast<ChatChannel>(d.read_u8());
        sender = d.read_string();
        target = d.read_string();
        content = d.read_string();
    }
};

// Player input
struct PlayerInputPayload {
    u32 tick;
    u32 last_received_tick;  // Last server tick the client has seen
    i8 move_x;
    i8 move_y;
    u8 buttons;              // Packed button flags
    Vec2i target_tile;

    void serialize(Serializer& s) const {
        s.write_u32(tick);
        s.write_u32(last_received_tick);
        s.write_i8(move_x);
        s.write_i8(move_y);
        s.write_u8(buttons);
        s.write_vec2i(target_tile);
    }

    void deserialize(Deserializer& d) {
        tick = d.read_u32();
        last_received_tick = d.read_u32();
        move_x = d.read_i8();
        move_y = d.read_i8();
        buttons = d.read_u8();
        target_tile = d.read_vec2i();
    }
};

// Entity spawn notification
struct EntitySpawnPayload {
    NetEntityId entity_id;
    Vec2f position;
    std::string name;       // Player name (empty for non-players)
    bool is_player;

    void serialize(Serializer& s) const {
        s.write_u32(entity_id);
        s.write_vec2f(position);
        s.write_string(name);
        s.write_bool(is_player);
    }

    void deserialize(Deserializer& d) {
        entity_id = d.read_u32();
        position = d.read_vec2f();
        name = d.read_string();
        is_player = d.read_bool();
    }
};

// Entity despawn notification
struct EntityDespawnPayload {
    NetEntityId entity_id;

    void serialize(Serializer& s) const {
        s.write_u32(entity_id);
    }

    void deserialize(Deserializer& d) {
        entity_id = d.read_u32();
    }
};

} // namespace city::net
