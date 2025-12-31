#pragma once

#include "core/content/content_manifest.hpp"
#include <functional>
#include <vector>

namespace city {

class ContentDownloader {
public:
    using ProgressCallback = std::function<void(f32 progress, const std::string& current_file)>;

    ContentDownloader() = default;

    // Start downloading content for a manifest
    void start_download(const ContentManifest& manifest, ProgressCallback on_progress = nullptr);

    // Process incoming content chunks
    void on_chunk_received(ResourceId id, u32 chunk_index, std::span<const u8> data);

    // Check if download is complete
    bool is_complete() const { return complete_; }

    // Get download progress (0.0 to 1.0)
    f32 progress() const;

private:
    ContentManifest manifest_;
    std::vector<ResourceId> pending_;
    u64 downloaded_bytes_{0};
    bool complete_{false};
    ProgressCallback on_progress_;
};

} // namespace city
