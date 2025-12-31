#include "client_session.hpp"
#include <enet/enet.h>

namespace city {

ClientSession::ClientSession(u32 id, void* peer)
    : id_(id), peer_(peer) {}

void ClientSession::send(net::Message msg, net::Reliability reliability) {
    if (!peer_) return;

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
    enet_peer_send(static_cast<ENetPeer*>(peer_), channel, packet);
}

void ClientSession::on_message(const net::Message& msg) {
    pending_messages_.push(msg);
}

} // namespace city
