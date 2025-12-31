#include <gtest/gtest.h>
#include <vector>

TEST(ExampleTest, VectorPushBack) {
    std::vector<int> vec;
    vec.push_back(42);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], 42);
}

TEST(ExampleTest, BasicAssertions) {
    EXPECT_EQ(2 + 2, 4);
    EXPECT_NE(1, 2);
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
}
