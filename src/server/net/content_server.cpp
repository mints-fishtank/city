#include "content_server.hpp"

namespace city {

ContentServer::ContentServer(const ContentManifest& manifest)
    : manifest_(manifest) {}

void ContentServer::start_transfer(ClientSession& /*session*/) {
    // TODO: Queue content transfer for this client
}

void ContentServer::update() {
    // TODO: Process pending transfers, send chunks
}

} // namespace city
