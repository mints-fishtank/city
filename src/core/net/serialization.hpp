#pragma once

#include "core/util/types.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <span>
#include <optional>
#include <stdexcept>
#include <cstring>

namespace city {

// Exception for deserialization errors
class DeserializeError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Binary serializer - writes data to a buffer
class Serializer {
public:
    Serializer() = default;
    explicit Serializer(size_t reserve_size) { buffer_.reserve(reserve_size); }

    // Primitives
    void write_u8(u8 v) { buffer_.push_back(v); }

    void write_u16(u16 v) {
        buffer_.push_back(static_cast<u8>(v >> 8));
        buffer_.push_back(static_cast<u8>(v));
    }

    void write_u32(u32 v) {
        buffer_.push_back(static_cast<u8>(v >> 24));
        buffer_.push_back(static_cast<u8>(v >> 16));
        buffer_.push_back(static_cast<u8>(v >> 8));
        buffer_.push_back(static_cast<u8>(v));
    }

    void write_u64(u64 v) {
        write_u32(static_cast<u32>(v >> 32));
        write_u32(static_cast<u32>(v));
    }

    void write_i8(i8 v) { write_u8(static_cast<u8>(v)); }
    void write_i16(i16 v) { write_u16(static_cast<u16>(v)); }
    void write_i32(i32 v) { write_u32(static_cast<u32>(v)); }
    void write_i64(i64 v) { write_u64(static_cast<u64>(v)); }

    void write_f32(f32 v) {
        u32 bits;
        std::memcpy(&bits, &v, sizeof(bits));
        write_u32(bits);
    }

    void write_f64(f64 v) {
        u64 bits;
        std::memcpy(&bits, &v, sizeof(bits));
        write_u64(bits);
    }

    void write_bool(bool v) { write_u8(v ? 1 : 0); }

    // Variable-length integer (for small values that are usually small)
    void write_varint(u64 v) {
        while (v >= 0x80) {
            buffer_.push_back(static_cast<u8>(v | 0x80));
            v >>= 7;
        }
        buffer_.push_back(static_cast<u8>(v));
    }

    // String (length-prefixed with varint)
    void write_string(std::string_view s) {
        write_varint(s.size());
        write_bytes(std::as_bytes(std::span{s}));
    }

    // Raw bytes
    void write_bytes(std::span<const std::byte> data) {
        buffer_.insert(buffer_.end(),
            reinterpret_cast<const u8*>(data.data()),
            reinterpret_cast<const u8*>(data.data() + data.size()));
    }

    void write_bytes(std::span<const u8> data) {
        buffer_.insert(buffer_.end(), data.begin(), data.end());
    }

    // Vector types
    void write_vec2f(Vec2f v) {
        write_f32(v.x);
        write_f32(v.y);
    }

    void write_vec2i(Vec2i v) {
        write_i32(v.x);
        write_i32(v.y);
    }

    // Output
    [[nodiscard]] std::span<const u8> data() const { return buffer_; }
    [[nodiscard]] size_t size() const { return buffer_.size(); }
    [[nodiscard]] bool empty() const { return buffer_.empty(); }

    std::vector<u8> take() { return std::move(buffer_); }
    void clear() { buffer_.clear(); }

private:
    std::vector<u8> buffer_;
};

// Binary deserializer - reads data from a buffer
class Deserializer {
public:
    explicit Deserializer(std::span<const u8> data) : data_(data) {}

    // Primitives
    u8 read_u8() {
        check_remaining(1);
        return data_[pos_++];
    }

    u16 read_u16() {
        check_remaining(2);
        u16 v = static_cast<u16>(
            (static_cast<u16>(data_[pos_]) << 8) |
            static_cast<u16>(data_[pos_ + 1])
        );
        pos_ += 2;
        return v;
    }

    u32 read_u32() {
        check_remaining(4);
        u32 v = (static_cast<u32>(data_[pos_]) << 24) |
                (static_cast<u32>(data_[pos_ + 1]) << 16) |
                (static_cast<u32>(data_[pos_ + 2]) << 8) |
                static_cast<u32>(data_[pos_ + 3]);
        pos_ += 4;
        return v;
    }

    u64 read_u64() {
        u64 high = read_u32();
        u64 low = read_u32();
        return (high << 32) | low;
    }

    i8 read_i8() { return static_cast<i8>(read_u8()); }
    i16 read_i16() { return static_cast<i16>(read_u16()); }
    i32 read_i32() { return static_cast<i32>(read_u32()); }
    i64 read_i64() { return static_cast<i64>(read_u64()); }

    f32 read_f32() {
        u32 bits = read_u32();
        f32 v;
        std::memcpy(&v, &bits, sizeof(v));
        return v;
    }

    f64 read_f64() {
        u64 bits = read_u64();
        f64 v;
        std::memcpy(&v, &bits, sizeof(v));
        return v;
    }

    bool read_bool() { return read_u8() != 0; }

    // Variable-length integer
    u64 read_varint() {
        u64 result = 0;
        u32 shift = 0;
        while (true) {
            if (shift >= 64) {
                throw DeserializeError("varint too large");
            }
            u8 b = read_u8();
            result |= static_cast<u64>(b & 0x7F) << shift;
            if ((b & 0x80) == 0) {
                break;
            }
            shift += 7;
        }
        return result;
    }

    // String
    std::string read_string() {
        size_t len = static_cast<size_t>(read_varint());
        check_remaining(len);
        std::string result(reinterpret_cast<const char*>(data_.data() + pos_), len);
        pos_ += len;
        return result;
    }

    // Raw bytes
    std::vector<u8> read_bytes(size_t len) {
        check_remaining(len);
        std::vector<u8> result(data_.begin() + static_cast<std::ptrdiff_t>(pos_),
                               data_.begin() + static_cast<std::ptrdiff_t>(pos_ + len));
        pos_ += len;
        return result;
    }

    // Read bytes into existing buffer
    void read_bytes_into(std::span<u8> out) {
        check_remaining(out.size());
        std::memcpy(out.data(), data_.data() + pos_, out.size());
        pos_ += out.size();
    }

    // Vector types
    Vec2f read_vec2f() {
        f32 x = read_f32();
        f32 y = read_f32();
        return {x, y};
    }

    Vec2i read_vec2i() {
        i32 x = read_i32();
        i32 y = read_i32();
        return {x, y};
    }

    // State
    [[nodiscard]] bool at_end() const { return pos_ >= data_.size(); }
    [[nodiscard]] size_t remaining() const { return data_.size() - pos_; }
    [[nodiscard]] size_t position() const { return pos_; }

    void skip(size_t bytes) {
        check_remaining(bytes);
        pos_ += bytes;
    }

private:
    void check_remaining(size_t needed) const {
        if (pos_ + needed > data_.size()) {
            throw DeserializeError("unexpected end of data");
        }
    }

    std::span<const u8> data_;
    size_t pos_ = 0;
};

// Concept for types that can be serialized
template<typename T>
concept Serializable = requires(T t, Serializer& s, Deserializer& d) {
    { t.serialize(s) } -> std::same_as<void>;
    { t.deserialize(d) } -> std::same_as<void>;
};

} // namespace city
