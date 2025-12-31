#pragma once

#include "content_manifest.hpp"
#include <filesystem>
#include <memory>
#include <unordered_map>

namespace city {

// Base class for loaded assets
class LoadedAsset {
public:
    virtual ~LoadedAsset() = default;
    virtual AssetType type() const = 0;
};

// Content loader - manages loading and caching assets
class ContentLoader {
public:
    explicit ContentLoader(std::filesystem::path cache_dir);

    // Load manifest
    void load_manifest(const ContentManifest& manifest);
    const ContentManifest& manifest() const { return manifest_; }

    // Check what needs downloading (compares with cache)
    std::vector<ResourceId> get_missing_assets() const;

    // Store downloaded asset to cache
    void store_asset(ResourceId id, std::span<const u8> data);

    // Load an asset by ID
    template<typename T>
    T* get(ResourceId id);

    // Load an asset by path
    template<typename T>
    T* get(std::string_view path) {
        return get<T>(ResourceId::from_path(path));
    }

    // Preload all assets of a type
    void preload(AssetType type);

    // Clear cached assets from memory
    void clear_cache();

private:
    std::filesystem::path cache_dir_;
    ContentManifest manifest_;
    std::unordered_map<ResourceId, std::unique_ptr<LoadedAsset>> loaded_;
    std::unordered_map<ResourceId, std::vector<u8>> raw_cache_;
};

} // namespace city
