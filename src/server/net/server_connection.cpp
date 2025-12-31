#include "server_connection.hpp"
#include "../server.hpp"
#include <enet/enet.h>
#include <iostream>

namespace city {

ServerConnection::ServerConnection(Server& server) : server_(server) {
    if (enet_initialize() != 0) {
        std::cerr << "Failed to initialize ENet\n";
    }
}

ServerConnection::~ServerConnection() {
    stop();
    enet_deinitialize();
}

bool ServerConnection::start(u16 port) {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    host_ = enet_host_create(&address, net::MAX_PLAYERS, 2, 0, 0);
    if (!host_) {
        std::cerr << "Failed to create ENet host on port " << port << "\n";
        return false;
    }

    return true;
}

void ServerConnection::stop() {
    sessions_.clear();
    if (host_) {
        enet_host_destroy(static_cast<ENetHost*>(host_));
        host_ = nullptr;
    }
}

void ServerConnection::update() {
    if (!host_) return;

    ENetEvent event;
    while (enet_host_service(static_cast<ENetHost*>(host_), &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                on_connect(event.peer);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                on_disconnect(event.peer);
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                on_receive(event.peer, event.packet->data, event.packet->dataLength);
                enet_packet_destroy(event.packet);
                break;

            default:
                break;
        }
    }
}

void ServerConnection::send(u32 session_id, net::Message msg, net::Reliability reliability) {
    auto* session = get_session(session_id);
    if (session) {
        session->send(std::move(msg), reliability);
    }
}

void ServerConnection::broadcast(net::Message msg, net::Reliability reliability) {
    auto data = msg.encode();

    u32 flags = 0;
    u8 channel = 0;

    switch (reliability) {
        case net::Reliability::Unreliable:
            flags = ENET_PACKET_FLAG_UNSEQUENCED;
            channel = 1;
            break;
        case net::Reliability::UnreliableSequenced:
            channel = 1;
            break;
        case net::Reliability::Reliable:
        case net::Reliability::ReliableOrdered:
            flags = ENET_PACKET_FLAG_RELIABLE;
            break;
    }

    ENetPacket* packet = enet_packet_create(data.data(), data.size(), flags);
    enet_host_broadcast(static_cast<ENetHost*>(host_), channel, packet);
}

ClientSession* ServerConnection::get_session(u32 session_id) {
    for (auto& session : sessions_) {
        if (session && session->id() == session_id) {
            return session.get();
        }
    }
    return nullptr;
}

u32 ServerConnection::client_count() const {
    u32 count = 0;
    for (const auto& session : sessions_) {
        if (session) ++count;
    }
    return count;
}

void ServerConnection::on_connect(void* peer) {
    auto* enet_peer = static_cast<ENetPeer*>(peer);

    // Create new session
    u32 session_id = next_session_id_++;
    auto session = std::make_unique<ClientSession>(session_id, peer);

    enet_peer->data = session.get();
    sessions_.push_back(std::move(session));

    std::cout << "Client connected (session " << session_id << ")\n";
}

void ServerConnection::on_disconnect(void* peer) {
    auto* enet_peer = static_cast<ENetPeer*>(peer);
    auto* session = static_cast<ClientSession*>(enet_peer->data);

    if (session) {
        std::cout << "Client disconnected (session " << session->id() << ")\n";

        // Find and remove session
        for (auto it = sessions_.begin(); it != sessions_.end(); ++it) {
            if (it->get() == session) {
                sessions_.erase(it);
                break;
            }
        }
    }

    enet_peer->data = nullptr;
}

void ServerConnection::on_receive(void* peer, const u8* data, size_t size) {
    auto* enet_peer = static_cast<ENetPeer*>(peer);
    auto* session = static_cast<ClientSession*>(enet_peer->data);

    if (!session) return;

    auto msg = net::Message::parse({data, size});
    if (!msg) return;

    // Handle client hello specially
    if (msg->type() == net::MessageType::ClientHello && session->state() == SessionState::Connected) {
        net::ClientHelloPayload hello;
        auto reader = msg->reader();
        hello.deserialize(reader);

        session->set_name(hello.player_name);
        session->set_state(SessionState::Ready);

        // Notify server
        // server_.on_client_connected(*session);
    }

    session->on_message(*msg);
}

} // namespace city
