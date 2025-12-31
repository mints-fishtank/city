#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>

namespace city {

// Common type aliases
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using f32 = float;
using f64 = double;

// 2D vector
template<typename T>
struct Vec2 {
    T x{};
    T y{};

    constexpr Vec2() = default;
    constexpr Vec2(T x_, T y_) : x(x_), y(y_) {}

    // Arithmetic operators
    constexpr Vec2 operator+(Vec2 other) const { return {x + other.x, y + other.y}; }
    constexpr Vec2 operator-(Vec2 other) const { return {x - other.x, y - other.y}; }
    constexpr Vec2 operator*(T scalar) const { return {x * scalar, y * scalar}; }
    constexpr Vec2 operator/(T scalar) const { return {x / scalar, y / scalar}; }

    constexpr Vec2& operator+=(Vec2 other) { x += other.x; y += other.y; return *this; }
    constexpr Vec2& operator-=(Vec2 other) { x -= other.x; y -= other.y; return *this; }
    constexpr Vec2& operator*=(T scalar) { x *= scalar; y *= scalar; return *this; }
    constexpr Vec2& operator/=(T scalar) { x /= scalar; y /= scalar; return *this; }

    constexpr Vec2 operator-() const { return {-x, -y}; }

    constexpr bool operator==(Vec2 other) const { return x == other.x && y == other.y; }
    constexpr bool operator!=(Vec2 other) const { return !(*this == other); }

    // Vector operations
    constexpr T dot(Vec2 other) const { return x * other.x + y * other.y; }
    constexpr T cross(Vec2 other) const { return x * other.y - y * other.x; }

    T length() const { return std::sqrt(x * x + y * y); }
    constexpr T length_squared() const { return x * x + y * y; }

    Vec2 normalized() const {
        T len = length();
        if (len > T(0)) {
            return *this / len;
        }
        return *this;
    }

    // Distance
    T distance(Vec2 other) const { return (*this - other).length(); }
    constexpr T distance_squared(Vec2 other) const { return (*this - other).length_squared(); }

    // Manhattan distance (for grid-based games)
    constexpr T manhattan_distance(Vec2 other) const {
        T dx = x > other.x ? x - other.x : other.x - x;
        T dy = y > other.y ? y - other.y : other.y - y;
        return dx + dy;
    }

    // Lerp
    constexpr Vec2 lerp(Vec2 target, T t) const {
        return *this + (target - *this) * t;
    }
};

// Common Vec2 types
using Vec2f = Vec2<f32>;
using Vec2i = Vec2<i32>;
using Vec2u = Vec2<u32>;

// Rectangle
template<typename T>
struct Rect {
    T x{};
    T y{};
    T width{};
    T height{};

    constexpr Rect() = default;
    constexpr Rect(T x_, T y_, T w_, T h_) : x(x_), y(y_), width(w_), height(h_) {}
    constexpr Rect(Vec2<T> pos, Vec2<T> size) : x(pos.x), y(pos.y), width(size.x), height(size.y) {}

    constexpr Vec2<T> position() const { return {x, y}; }
    constexpr Vec2<T> size() const { return {width, height}; }
    constexpr Vec2<T> center() const { return {x + width / T(2), y + height / T(2)}; }

    constexpr T left() const { return x; }
    constexpr T right() const { return x + width; }
    constexpr T top() const { return y; }
    constexpr T bottom() const { return y + height; }

    constexpr bool contains(Vec2<T> point) const {
        return point.x >= x && point.x < x + width &&
               point.y >= y && point.y < y + height;
    }

    constexpr bool intersects(Rect other) const {
        return x < other.x + other.width && x + width > other.x &&
               y < other.y + other.height && y + height > other.y;
    }

    constexpr Rect intersection(Rect other) const {
        T ix = std::max(x, other.x);
        T iy = std::max(y, other.y);
        T iw = std::min(right(), other.right()) - ix;
        T ih = std::min(bottom(), other.bottom()) - iy;
        if (iw <= T(0) || ih <= T(0)) {
            return {};
        }
        return {ix, iy, iw, ih};
    }

    constexpr bool operator==(Rect other) const {
        return x == other.x && y == other.y && width == other.width && height == other.height;
    }
};

// Common Rect types
using Rectf = Rect<f32>;
using Recti = Rect<i32>;

// Color (RGBA, 0-255)
struct Color {
    u8 r{255};
    u8 g{255};
    u8 b{255};
    u8 a{255};

    constexpr Color() = default;
    constexpr Color(u8 r_, u8 g_, u8 b_, u8 a_ = 255) : r(r_), g(g_), b(b_), a(a_) {}

    // Predefined colors
    static constexpr Color white() { return {255, 255, 255}; }
    static constexpr Color black() { return {0, 0, 0}; }
    static constexpr Color red() { return {255, 0, 0}; }
    static constexpr Color green() { return {0, 255, 0}; }
    static constexpr Color blue() { return {0, 0, 255}; }
    static constexpr Color yellow() { return {255, 255, 0}; }
    static constexpr Color cyan() { return {0, 255, 255}; }
    static constexpr Color magenta() { return {255, 0, 255}; }
    static constexpr Color transparent() { return {0, 0, 0, 0}; }

    constexpr bool operator==(Color other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    // Pack to u32 (RGBA)
    constexpr u32 to_u32() const {
        return (static_cast<u32>(r) << 24) |
               (static_cast<u32>(g) << 16) |
               (static_cast<u32>(b) << 8) |
               static_cast<u32>(a);
    }

    static constexpr Color from_u32(u32 value) {
        return {
            static_cast<u8>((value >> 24) & 0xFF),
            static_cast<u8>((value >> 16) & 0xFF),
            static_cast<u8>((value >> 8) & 0xFF),
            static_cast<u8>(value & 0xFF)
        };
    }
};

} // namespace city
