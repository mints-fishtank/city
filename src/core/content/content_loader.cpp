#include "content_loader.hpp"

namespace city {

ContentLoader::ContentLoader(std::filesystem::path cache_dir)
    : cache_dir_(std::move(cache_dir)) {
    std::filesystem::create_directories(cache_dir_);
}

void ContentLoader::load_manifest(const ContentManifest& manifest) {
    manifest_ = manifest;
}

std::vector<ResourceId> ContentLoader::get_missing_assets() const {
    std::vector<ResourceId> missing;
    for (const auto& asset : manifest_.assets) {
        // Check if asset exists in cache
        auto cache_path = cache_dir_ / manifest_.server_id / asset.path;
        if (!std::filesystem::exists(cache_path)) {
            missing.push_back(asset.id);
        }
    }
    return missing;
}

void ContentLoader::store_asset(ResourceId id, std::span<const u8> data) {
    const auto* entry = manifest_.find(id);
    if (!entry) return;

    auto cache_path = cache_dir_ / manifest_.server_id / entry->path;
    std::filesystem::create_directories(cache_path.parent_path());

    // TODO: Write data to file
    raw_cache_[id] = std::vector<u8>(data.begin(), data.end());
}

void ContentLoader::preload(AssetType /*type*/) {
    // TODO: Implement preloading
}

void ContentLoader::clear_cache() {
    loaded_.clear();
    raw_cache_.clear();
}

} // namespace city
