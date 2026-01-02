#include "input_buffer.hpp"

namespace city {

void InputBuffer::add(InputSnapshot input) {
    size_t index = input.tick % BUFFER_SIZE;

    // Track the oldest tick in the buffer
    if (oldest_tick_ == 0) {
        oldest_tick_ = input.tick;
    }

    // If we're overwriting an old input, update oldest_tick
    if (buffer_[index].has_value() && buffer_[index]->tick < input.tick) {
        // The slot we're overwriting had old data, find new oldest
        oldest_tick_ = input.tick > BUFFER_SIZE ? input.tick - BUFFER_SIZE + 1 : 1;
    }

    buffer_[index] = input;
    latest_tick_ = input.tick;
}

void InputBuffer::clear() {
    for (auto& entry : buffer_) {
        entry = std::nullopt;
    }
    last_acked_tick_ = 0;
    latest_tick_ = 0;
    oldest_tick_ = 0;
}

void InputBuffer::acknowledge(u32 tick) {
    // Just update the acknowledged tick - we keep inputs for resimulation
    // Old inputs will naturally be overwritten as the buffer wraps
    if (tick > last_acked_tick_) {
        last_acked_tick_ = tick;
    }
}

std::optional<InputSnapshot> InputBuffer::get(u32 tick) const {
    if (tick < oldest_tick_ || tick > latest_tick_) {
        return std::nullopt;
    }
    auto& entry = buffer_[tick % BUFFER_SIZE];
    // Verify this slot actually contains the tick we want (not wrapped)
    if (entry.has_value() && entry->tick == tick) {
        return entry;
    }
    return std::nullopt;
}

std::vector<InputSnapshot> InputBuffer::get_inputs_after(u32 tick) const {
    std::vector<InputSnapshot> result;
    // Get all inputs from tick+1 to latest_tick
    for (u32 t = tick + 1; t <= latest_tick_; ++t) {
        if (auto input = get(t)) {
            result.push_back(*input);
        }
    }
    return result;
}

std::vector<InputSnapshot> InputBuffer::get_unacknowledged() const {
    return get_inputs_after(last_acked_tick_);
}

} // namespace city
