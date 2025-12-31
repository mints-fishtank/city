#pragma once

#include "core/net/protocol.hpp"
#include "core/net/message.hpp"
#include <string>
#include <queue>
#include <optional>

namespace city {

enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Disconnecting,
};

class ClientConnection {
public:
    ClientConnection();
    ~ClientConnection();

    bool connect(const std::string& host, u16 port);
    void disconnect();
    void update();

    void send(net::Message msg, net::Reliability reliability = net::Reliability::ReliableOrdered);
    std::optional<net::Message> receive();

    ConnectionState state() const { return state_; }
    u32 ping_ms() const { return ping_ms_; }

private:
    ConnectionState state_{ConnectionState::Disconnected};
    u32 ping_ms_{0};

    std::queue<net::Message> incoming_;

    // ENet peer handle (to be implemented)
    void* peer_{nullptr};
    void* host_{nullptr};
};

} // namespace city
