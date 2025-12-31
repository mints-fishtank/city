#include "client_connection.hpp"
#include <enet/enet.h>
#include <iostream>

namespace city {

ClientConnection::ClientConnection() {
    if (enet_initialize() != 0) {
        std::cerr << "Failed to initialize ENet\n";
    }
}

ClientConnection::~ClientConnection() {
    disconnect();
    enet_deinitialize();
}

bool ClientConnection::connect(const std::string& host, u16 port) {
    if (state_ != ConnectionState::Disconnected) {
        return false;
    }

    host_ = enet_host_create(nullptr, 1, 2, 0, 0);
    if (!host_) {
        std::cerr << "Failed to create ENet host\n";
        return false;
    }

    ENetAddress address;
    enet_address_set_host(&address, host.c_str());
    address.port = port;

    peer_ = enet_host_connect(static_cast<ENetHost*>(host_), &address, 2, 0);
    if (!peer_) {
        std::cerr << "Failed to connect to " << host << ":" << port << "\n";
        enet_host_destroy(static_cast<ENetHost*>(host_));
        host_ = nullptr;
        return false;
    }

    state_ = ConnectionState::Connecting;
    std::cout << "Connecting to " << host << ":" << port << "...\n";
    return true;
}

void ClientConnection::disconnect() {
    if (peer_) {
        enet_peer_disconnect(static_cast<ENetPeer*>(peer_), 0);
        peer_ = nullptr;
    }
    if (host_) {
        enet_host_destroy(static_cast<ENetHost*>(host_));
        host_ = nullptr;
    }
    state_ = ConnectionState::Disconnected;
}

void ClientConnection::update() {
    if (!host_) return;

    ENetEvent event;
    while (enet_host_service(static_cast<ENetHost*>(host_), &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                std::cout << "Connected to server\n";
                state_ = ConnectionState::Connected;
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                std::cout << "Disconnected from server\n";
                state_ = ConnectionState::Disconnected;
                peer_ = nullptr;
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                if (auto msg = net::Message::parse({event.packet->data, event.packet->dataLength})) {
                    incoming_.push(std::move(*msg));
                }
                enet_packet_destroy(event.packet);
                break;

            default:
                break;
        }
    }

    // Update ping
    if (peer_) {
        ping_ms_ = static_cast<ENetPeer*>(peer_)->roundTripTime;
    }
}

void ClientConnection::send(net::Message msg, net::Reliability reliability) {
    if (!peer_ || state_ != ConnectionState::Connected) return;

    auto data = msg.encode();
    u32 flags = 0;
    u8 channel = 0;

    switch (reliability) {
        case net::Reliability::Unreliable:
            flags = ENET_PACKET_FLAG_UNSEQUENCED;
            channel = 1;
            break;
        case net::Reliability::UnreliableSequenced:
            flags = 0;
            channel = 1;
            break;
        case net::Reliability::Reliable:
            flags = ENET_PACKET_FLAG_RELIABLE;
            channel = 0;
            break;
        case net::Reliability::ReliableOrdered:
            flags = ENET_PACKET_FLAG_RELIABLE;
            channel = 0;
            break;
    }

    ENetPacket* packet = enet_packet_create(data.data(), data.size(), flags);
    enet_peer_send(static_cast<ENetPeer*>(peer_), channel, packet);
}

std::optional<net::Message> ClientConnection::receive() {
    if (incoming_.empty()) return std::nullopt;
    auto msg = std::move(incoming_.front());
    incoming_.pop();
    return msg;
}

} // namespace city
