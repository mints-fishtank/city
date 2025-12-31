#pragma once

#include "core/content/content_manifest.hpp"
#include "client_session.hpp"

namespace city {

class ContentServer {
public:
    explicit ContentServer(const ContentManifest& manifest);

    // Start sending content to a client
    void start_transfer(ClientSession& session);

    // Process pending transfers
    void update();

private:
    const ContentManifest& manifest_;
};

} // namespace city
