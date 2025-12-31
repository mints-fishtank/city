#pragma once

#include "entity.hpp"
#include <vector>
#include <optional>
#include <type_traits>
#include <typeindex>

namespace city {

// Forward declarations
class Serializer;
class Deserializer;

// Component type ID - assigned at runtime per component type
using ComponentTypeId = u16;
constexpr ComponentTypeId INVALID_COMPONENT_TYPE_ID = std::numeric_limits<u16>::max();

// Sparse set component storage
// Provides O(1) access, add, remove while maintaining cache-friendly dense array
template<typename T>
class ComponentPool {
public:
    // Get component for entity (returns nullptr if not present)
    T* get(u32 entity_index) {
        if (entity_index >= sparse_.size()) {
            return nullptr;
        }
        u32 dense_index = sparse_[entity_index];
        if (dense_index == INVALID_INDEX) {
            return nullptr;
        }
        return &dense_[dense_index].component;
    }

    const T* get(u32 entity_index) const {
        if (entity_index >= sparse_.size()) {
            return nullptr;
        }
        u32 dense_index = sparse_[entity_index];
        if (dense_index == INVALID_INDEX) {
            return nullptr;
        }
        return &dense_[dense_index].component;
    }

    // Check if entity has this component
    bool has(u32 entity_index) const {
        return entity_index < sparse_.size() && sparse_[entity_index] != INVALID_INDEX;
    }

    // Add or replace component for entity
    T& set(u32 entity_index, T component) {
        // Ensure sparse array is large enough
        if (entity_index >= sparse_.size()) {
            sparse_.resize(entity_index + 1, INVALID_INDEX);
        }

        u32& dense_index = sparse_[entity_index];
        if (dense_index != INVALID_INDEX) {
            // Replace existing
            dense_[dense_index].component = std::move(component);
            return dense_[dense_index].component;
        }

        // Add new
        dense_index = static_cast<u32>(dense_.size());
        dense_.push_back({entity_index, std::move(component)});
        return dense_.back().component;
    }

    // Remove component from entity
    void remove(u32 entity_index) {
        if (entity_index >= sparse_.size()) {
            return;
        }

        u32 dense_index = sparse_[entity_index];
        if (dense_index == INVALID_INDEX) {
            return;
        }

        // Swap with last element for O(1) removal
        u32 last_dense_index = static_cast<u32>(dense_.size() - 1);
        if (dense_index != last_dense_index) {
            // Move last element to removed position
            dense_[dense_index] = std::move(dense_[last_dense_index]);
            // Update sparse array for moved element
            sparse_[dense_[dense_index].entity_index] = dense_index;
        }

        dense_.pop_back();
        sparse_[entity_index] = INVALID_INDEX;
    }

    // Iteration support
    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<u32, T&>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type;

        ComponentPool* pool;
        size_t index;

        reference operator*() const {
            return {pool->dense_[index].entity_index, pool->dense_[index].component};
        }

        Iterator& operator++() { ++index; return *this; }
        Iterator operator++(int) { Iterator tmp = *this; ++index; return tmp; }

        bool operator==(const Iterator& other) const { return index == other.index; }
        bool operator!=(const Iterator& other) const { return index != other.index; }
    };

    Iterator begin() { return {this, 0}; }
    Iterator end() { return {this, dense_.size()}; }

    // Size
    size_t size() const { return dense_.size(); }
    bool empty() const { return dense_.empty(); }

    // Clear all components
    void clear() {
        sparse_.clear();
        dense_.clear();
    }

private:
    static constexpr u32 INVALID_INDEX = std::numeric_limits<u32>::max();

    struct Entry {
        u32 entity_index;
        T component;
    };

    std::vector<u32> sparse_;     // entity_index -> dense_index
    std::vector<Entry> dense_;    // Packed component storage
};

} // namespace city
