#include <gtest/gtest.h>
#include "core/grid/tilemap.hpp"

using namespace city;

TEST(Grid, TilePos) {
    TilePos p1{10, 20};
    TilePos p2{15, 25};

    EXPECT_EQ(p1.manhattan_distance(p2), 10);
    EXPECT_EQ(p1.chebyshev_distance(p2), 5);

    TilePos sum = p1 + p2;
    EXPECT_EQ(sum.x, 25);
    EXPECT_EQ(sum.y, 45);
}

TEST(Grid, TilePosFromWorld) {
    Vec2f world_pos{10.5f, 20.7f};
    TilePos tile = TilePos::from_world(world_pos);

    EXPECT_EQ(tile.x, 10);
    EXPECT_EQ(tile.y, 20);

    // Negative coordinates
    Vec2f negative{-0.5f, -1.5f};
    TilePos neg_tile = TilePos::from_world(negative);
    EXPECT_EQ(neg_tile.x, -1);
    EXPECT_EQ(neg_tile.y, -2);
}

TEST(Grid, ChunkOrigin) {
    // Positive coordinates
    EXPECT_EQ(Chunk::get_chunk_origin({0, 0}), TilePos(0, 0));
    EXPECT_EQ(Chunk::get_chunk_origin({15, 15}), TilePos(0, 0));
    EXPECT_EQ(Chunk::get_chunk_origin({16, 16}), TilePos(16, 16));
    EXPECT_EQ(Chunk::get_chunk_origin({31, 31}), TilePos(16, 16));

    // Negative coordinates
    EXPECT_EQ(Chunk::get_chunk_origin({-1, -1}), TilePos(-16, -16));
    EXPECT_EQ(Chunk::get_chunk_origin({-16, -16}), TilePos(-16, -16));
    EXPECT_EQ(Chunk::get_chunk_origin({-17, -17}), TilePos(-32, -32));
}

TEST(Grid, TileMapSetGet) {
    TileMap map;

    Tile tile;
    tile.floor_id = 1;
    tile.wall_id = 2;
    tile.flags = TileFlags::Solid;

    map.set_tile({5, 10}, tile);

    const Tile* retrieved = map.get_tile({5, 10});
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(retrieved->floor_id, 1);
    EXPECT_EQ(retrieved->wall_id, 2);
    EXPECT_FALSE(retrieved->is_passable());

    // Non-existent tile
    EXPECT_EQ(map.get_tile({100, 100}), nullptr);
}

TEST(Grid, TileMapBounds) {
    TileMap map;
    map.set_bounds(100, 100);

    EXPECT_TRUE(map.in_bounds({0, 0}));
    EXPECT_TRUE(map.in_bounds({99, 99}));
    EXPECT_FALSE(map.in_bounds({-1, 0}));
    EXPECT_FALSE(map.in_bounds({100, 0}));
}

TEST(Grid, TileMapPassable) {
    TileMap map;

    // Set up some tiles
    Tile floor;
    floor.floor_id = 1;

    Tile wall;
    wall.floor_id = 1;
    wall.wall_id = 1;
    wall.flags = TileFlags::Solid;

    map.set_tile({0, 0}, floor);
    map.set_tile({1, 0}, wall);
    map.set_tile({0, 1}, floor);

    EXPECT_TRUE(map.is_passable({0, 0}));
    EXPECT_FALSE(map.is_passable({1, 0}));
    EXPECT_TRUE(map.is_passable({0, 1}));
    EXPECT_FALSE(map.is_passable({5, 5}));  // Non-existent
}

TEST(Grid, PassableNeighbors) {
    TileMap map;

    Tile floor;
    floor.floor_id = 1;

    Tile wall;
    wall.flags = TileFlags::Solid;

    // Create a small area
    // F F W
    // F F F
    // W F F
    map.set_tile({0, 0}, floor); map.set_tile({1, 0}, floor); map.set_tile({2, 0}, wall);
    map.set_tile({0, 1}, floor); map.set_tile({1, 1}, floor); map.set_tile({2, 1}, floor);
    map.set_tile({0, 2}, wall);  map.set_tile({1, 2}, floor); map.set_tile({2, 2}, floor);

    auto neighbors = map.get_passable_neighbors({1, 1}, false);
    EXPECT_EQ(neighbors.size(), 4);  // All cardinal directions are passable
}

TEST(Grid, LineOfSight) {
    auto line = TileMap::get_line({0, 0}, {3, 2});

    EXPECT_EQ(line.size(), 4);
    EXPECT_EQ(line[0], TilePos(0, 0));
    EXPECT_EQ(line.back(), TilePos(3, 2));
}

TEST(Grid, Serialization) {
    TileMap map;

    Tile tile;
    tile.floor_id = 42;
    tile.wall_id = 7;
    tile.flags = TileFlags::Solid | TileFlags::Opaque;

    map.set_tile({5, 10}, tile);
    map.set_tile({20, 30}, tile);

    // Serialize
    Serializer s;
    map.serialize(s);

    // Deserialize into new map
    TileMap map2;
    Deserializer d(s.data());
    map2.deserialize(d);

    // Verify
    const Tile* t1 = map2.get_tile({5, 10});
    ASSERT_NE(t1, nullptr);
    EXPECT_EQ(t1->floor_id, 42);
    EXPECT_EQ(t1->wall_id, 7);

    const Tile* t2 = map2.get_tile({20, 30});
    ASSERT_NE(t2, nullptr);
    EXPECT_EQ(t2->floor_id, 42);
}
