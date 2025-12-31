#pragma once

#include "core/util/types.hpp"
#include "core/net/serialization.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace city {

// Asset types
enum class AssetType : u8 {
    Sprite,
    SpriteSheet,
    TileSet,
    Map,
    EntityDef,
    ItemDef,
    Sound,
    Script,
};

// Resource identifier (hash of path)
struct ResourceId {
    u64 hash{0};

    static ResourceId from_path(std::string_view path);

    bool operator==(ResourceId other) const { return hash == other.hash; }
    bool operator!=(ResourceId other) const { return hash != other.hash; }
};

// Single asset entry in manifest
struct AssetEntry {
    ResourceId id;
    AssetType type;
    std::string path;
    u64 size;
    u64 checksum;

    void serialize(Serializer& s) const;
    void deserialize(Deserializer& d);
};

// Content manifest - describes all assets for a server
class ContentManifest {
public:
    std::string server_id;
    std::string server_name;
    u32 version{0};
    u64 total_size{0};
    std::vector<AssetEntry> assets;

    // Find asset by ID
    const AssetEntry* find(ResourceId id) const;

    // Find assets by type
    std::vector<const AssetEntry*> find_by_type(AssetType type) const;

    // Generate manifest from directory
    static ContentManifest from_directory(const std::string& path, std::string_view server_id);

    void serialize(Serializer& s) const;
    void deserialize(Deserializer& d);
};

} // namespace city

// Hash support
template<>
struct std::hash<city::ResourceId> {
    size_t operator()(city::ResourceId id) const noexcept {
        return std::hash<std::uint64_t>{}(id.hash);
    }
};
