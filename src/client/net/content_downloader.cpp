#include "content_downloader.hpp"

namespace city {

void ContentDownloader::start_download(const ContentManifest& manifest, ProgressCallback on_progress) {
    manifest_ = manifest;
    on_progress_ = std::move(on_progress);
    complete_ = false;
    downloaded_bytes_ = 0;

    // Build list of assets to download
    pending_.clear();
    for (const auto& asset : manifest_.assets) {
        pending_.push_back(asset.id);
    }

    if (pending_.empty()) {
        complete_ = true;
    }
}

void ContentDownloader::on_chunk_received(ResourceId id, u32 /*chunk_index*/, std::span<const u8> data) {
    downloaded_bytes_ += data.size();

    // TODO: Store chunk to disk

    // Check if this asset is complete
    // For now, assume single chunk per asset
    auto it = std::find(pending_.begin(), pending_.end(), id);
    if (it != pending_.end()) {
        pending_.erase(it);
    }

    if (pending_.empty()) {
        complete_ = true;
    }

    if (on_progress_) {
        on_progress_(progress(), "");
    }
}

f32 ContentDownloader::progress() const {
    if (manifest_.total_size == 0) return 1.0f;
    return static_cast<f32>(downloaded_bytes_) / static_cast<f32>(manifest_.total_size);
}

} // namespace city
