#pragma once

#include "client_session.hpp"
#include "core/net/message.hpp"
#include <vector>
#include <memory>
#include <functional>

namespace city {

class Server;

class ServerConnection {
public:
    explicit ServerConnection(Server& server);
    ~ServerConnection();

    bool start(u16 port);
    void stop();
    void update();

    // Send to specific client
    void send(u32 session_id, net::Message msg, net::Reliability reliability = net::Reliability::ReliableOrdered);

    // Broadcast to all clients
    void broadcast(net::Message msg, net::Reliability reliability = net::Reliability::ReliableOrdered);

    // Get client session
    ClientSession* get_session(u32 session_id);

    // Iterate all sessions
    template<typename Func>
    void for_each_session(Func&& func) {
        for (auto& session : sessions_) {
            if (session) {
                func(*session);
            }
        }
    }

    u32 client_count() const;

private:
    void on_connect(void* peer);
    void on_disconnect(void* peer);
    void on_receive(void* peer, const u8* data, size_t size);

    Server& server_;
    void* host_{nullptr};
    std::vector<std::unique_ptr<ClientSession>> sessions_;
    u32 next_session_id_{1};
    bool enet_initialized_{false};
};

} // namespace city
