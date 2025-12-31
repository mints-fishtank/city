#include "content_manifest.hpp"
#include <functional>

namespace city {

ResourceId ResourceId::from_path(std::string_view path) {
    // FNV-1a hash
    u64 hash = 14695981039346656037ULL;
    for (char c : path) {
        hash ^= static_cast<u64>(c);
        hash *= 1099511628211ULL;
    }
    return {hash};
}

void AssetEntry::serialize(Serializer& s) const {
    s.write_u64(id.hash);
    s.write_u8(static_cast<u8>(type));
    s.write_string(path);
    s.write_u64(size);
    s.write_u64(checksum);
}

void AssetEntry::deserialize(Deserializer& d) {
    id.hash = d.read_u64();
    type = static_cast<AssetType>(d.read_u8());
    path = d.read_string();
    size = d.read_u64();
    checksum = d.read_u64();
}

const AssetEntry* ContentManifest::find(ResourceId id) const {
    for (const auto& asset : assets) {
        if (asset.id == id) return &asset;
    }
    return nullptr;
}

std::vector<const AssetEntry*> ContentManifest::find_by_type(AssetType type) const {
    std::vector<const AssetEntry*> result;
    for (const auto& asset : assets) {
        if (asset.type == type) {
            result.push_back(&asset);
        }
    }
    return result;
}

ContentManifest ContentManifest::from_directory(const std::string& /*path*/, std::string_view server_id) {
    ContentManifest manifest;
    manifest.server_id = std::string(server_id);
    manifest.version = 1;
    // TODO: Scan directory and populate assets
    return manifest;
}

void ContentManifest::serialize(Serializer& s) const {
    s.write_string(server_id);
    s.write_string(server_name);
    s.write_u32(version);
    s.write_u64(total_size);
    s.write_u32(static_cast<u32>(assets.size()));
    for (const auto& asset : assets) {
        asset.serialize(s);
    }
}

void ContentManifest::deserialize(Deserializer& d) {
    server_id = d.read_string();
    server_name = d.read_string();
    version = d.read_u32();
    total_size = d.read_u64();
    u32 count = d.read_u32();
    assets.resize(count);
    for (auto& asset : assets) {
        asset.deserialize(d);
    }
}

} // namespace city
