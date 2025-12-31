#include <gtest/gtest.h>
#include "core/ecs/world.hpp"

using namespace city;

struct TestComponent {
    int value;
};

struct OtherComponent {
    float x, y;
};

TEST(ECS, CreateEntity) {
    World world;

    Entity e1 = world.create();
    Entity e2 = world.create();

    EXPECT_TRUE(e1.is_valid());
    EXPECT_TRUE(e2.is_valid());
    EXPECT_NE(e1, e2);
    EXPECT_TRUE(world.is_alive(e1));
    EXPECT_TRUE(world.is_alive(e2));
}

TEST(ECS, DestroyEntity) {
    World world;

    Entity e = world.create();
    EXPECT_TRUE(world.is_alive(e));

    world.destroy(e);
    EXPECT_FALSE(world.is_alive(e));
}

TEST(ECS, EntityReuse) {
    World world;

    Entity e1 = world.create();
    u32 index = e1.index;
    world.destroy(e1);

    Entity e2 = world.create();
    // Index should be reused
    EXPECT_EQ(e2.index, index);
    // But generation should be different
    EXPECT_NE(e2.generation, e1.generation);
    // Old entity reference should not be valid
    EXPECT_FALSE(world.is_alive(e1));
    EXPECT_TRUE(world.is_alive(e2));
}

TEST(ECS, AddComponent) {
    World world;
    Entity e = world.create();

    world.add_component<TestComponent>(e, {42});

    EXPECT_TRUE(world.has_component<TestComponent>(e));
    EXPECT_FALSE(world.has_component<OtherComponent>(e));

    auto* comp = world.get_component<TestComponent>(e);
    ASSERT_NE(comp, nullptr);
    EXPECT_EQ(comp->value, 42);
}

TEST(ECS, RemoveComponent) {
    World world;
    Entity e = world.create();

    world.add_component<TestComponent>(e, {42});
    EXPECT_TRUE(world.has_component<TestComponent>(e));

    world.remove_component<TestComponent>(e);
    EXPECT_FALSE(world.has_component<TestComponent>(e));
    EXPECT_EQ(world.get_component<TestComponent>(e), nullptr);
}

TEST(ECS, MultipleComponents) {
    World world;
    Entity e = world.create();

    world.add_component<TestComponent>(e, {42});
    world.add_component<OtherComponent>(e, {1.0f, 2.0f});

    EXPECT_TRUE(world.has_component<TestComponent>(e));
    EXPECT_TRUE(world.has_component<OtherComponent>(e));

    auto* test = world.get_component<TestComponent>(e);
    auto* other = world.get_component<OtherComponent>(e);

    ASSERT_NE(test, nullptr);
    ASSERT_NE(other, nullptr);
    EXPECT_EQ(test->value, 42);
    EXPECT_FLOAT_EQ(other->x, 1.0f);
    EXPECT_FLOAT_EQ(other->y, 2.0f);
}

TEST(ECS, NetworkId) {
    World world;
    Entity e = world.create();

    NetEntityId net_id = world.allocate_net_id();
    world.assign_net_id(e, net_id);

    EXPECT_EQ(world.get_net_id(e), net_id);
    EXPECT_EQ(world.get_by_net_id(net_id), e);
}

TEST(ECS, EachIteration) {
    World world;

    // Create entities with different component combinations
    Entity e1 = world.create();
    Entity e2 = world.create();
    Entity e3 = world.create();

    world.add_component<TestComponent>(e1, {1});
    world.add_component<TestComponent>(e2, {2});
    world.add_component<OtherComponent>(e2, {0.0f, 0.0f});
    world.add_component<OtherComponent>(e3, {0.0f, 0.0f});

    // Count entities with TestComponent
    int count = 0;
    int sum = 0;
    world.each<TestComponent>([&count, &sum](Entity, TestComponent& tc) {
        ++count;
        sum += tc.value;
    });

    EXPECT_EQ(count, 2);
    EXPECT_EQ(sum, 3);
}
