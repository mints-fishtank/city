#include <gtest/gtest.h>
#include "core/net/serialization.hpp"

using namespace city;

TEST(Serialization, Primitives) {
    Serializer s;
    s.write_u8(42);
    s.write_u16(1234);
    s.write_u32(0xDEADBEEF);
    s.write_i32(-12345);
    s.write_f32(3.14159f);
    s.write_bool(true);
    s.write_bool(false);

    Deserializer d(s.data());
    EXPECT_EQ(d.read_u8(), 42);
    EXPECT_EQ(d.read_u16(), 1234);
    EXPECT_EQ(d.read_u32(), 0xDEADBEEF);
    EXPECT_EQ(d.read_i32(), -12345);
    EXPECT_FLOAT_EQ(d.read_f32(), 3.14159f);
    EXPECT_TRUE(d.read_bool());
    EXPECT_FALSE(d.read_bool());
    EXPECT_TRUE(d.at_end());
}

TEST(Serialization, Strings) {
    Serializer s;
    s.write_string("Hello, World!");
    s.write_string("");
    s.write_string("Unicode: \xC3\xA9\xC3\xA0\xC3\xB9");  // UTF-8

    Deserializer d(s.data());
    EXPECT_EQ(d.read_string(), "Hello, World!");
    EXPECT_EQ(d.read_string(), "");
    EXPECT_EQ(d.read_string(), "Unicode: \xC3\xA9\xC3\xA0\xC3\xB9");
    EXPECT_TRUE(d.at_end());
}

TEST(Serialization, Varint) {
    Serializer s;
    s.write_varint(0);
    s.write_varint(127);
    s.write_varint(128);
    s.write_varint(16383);
    s.write_varint(16384);
    s.write_varint(0xFFFFFFFFFFFFFFFF);

    Deserializer d(s.data());
    EXPECT_EQ(d.read_varint(), 0u);
    EXPECT_EQ(d.read_varint(), 127u);
    EXPECT_EQ(d.read_varint(), 128u);
    EXPECT_EQ(d.read_varint(), 16383u);
    EXPECT_EQ(d.read_varint(), 16384u);
    EXPECT_EQ(d.read_varint(), 0xFFFFFFFFFFFFFFFF);
    EXPECT_TRUE(d.at_end());
}

TEST(Serialization, Vectors) {
    Serializer s;
    s.write_vec2f({3.14f, 2.71f});
    s.write_vec2i({-100, 200});

    Deserializer d(s.data());
    auto v2f = d.read_vec2f();
    EXPECT_FLOAT_EQ(v2f.x, 3.14f);
    EXPECT_FLOAT_EQ(v2f.y, 2.71f);

    auto v2i = d.read_vec2i();
    EXPECT_EQ(v2i.x, -100);
    EXPECT_EQ(v2i.y, 200);
    EXPECT_TRUE(d.at_end());
}

TEST(Serialization, UnexpectedEnd) {
    Serializer s;
    s.write_u8(42);

    Deserializer d(s.data());
    d.read_u8();
    EXPECT_THROW(d.read_u8(), DeserializeError);
}
