#pragma once

#include "core/net/message.hpp"
#include "core/ecs/entity.hpp"
#include <string>
#include <queue>

namespace city {

enum class SessionState {
    Connected,      // Just connected, awaiting hello
    Ready,          // Hello received, ready to play
    Playing,        // In-game
    Disconnecting,
};

class ClientSession {
public:
    ClientSession(u32 id, void* peer);

    u32 id() const { return id_; }
    const std::string& name() const { return name_; }
    SessionState state() const { return state_; }
    NetEntityId player_entity() const { return player_entity_; }

    void set_name(const std::string& name) { name_ = name; }
    void set_state(SessionState state) { state_ = state; }
    void set_player_entity(NetEntityId id) { player_entity_ = id; }

    void send(net::Message msg, net::Reliability reliability = net::Reliability::ReliableOrdered);

    void on_message(const net::Message& msg);

    // Get and clear pending messages
    std::queue<net::Message>& pending_messages() { return pending_messages_; }

private:
    u32 id_;
    void* peer_;
    std::string name_{"Player"};
    SessionState state_{SessionState::Connected};
    NetEntityId player_entity_{INVALID_NET_ENTITY_ID};
    std::queue<net::Message> pending_messages_;
};

} // namespace city
