#include "input_buffer.hpp"

namespace city {

void InputBuffer::add(InputSnapshot input) {
    size_t index = input.tick % BUFFER_SIZE;
    buffer_[index] = input;
    latest_tick_ = input.tick;
}

void InputBuffer::acknowledge(u32 tick) {
    // Clear acknowledged inputs
    while (last_acked_tick_ < tick) {
        size_t index = last_acked_tick_ % BUFFER_SIZE;
        buffer_[index] = std::nullopt;
        ++last_acked_tick_;
    }
}

std::optional<InputSnapshot> InputBuffer::get(u32 tick) const {
    if (tick <= last_acked_tick_ || tick > latest_tick_) {
        return std::nullopt;
    }
    return buffer_[tick % BUFFER_SIZE];
}

std::vector<InputSnapshot> InputBuffer::get_unacknowledged() const {
    std::vector<InputSnapshot> result;
    for (u32 tick = last_acked_tick_ + 1; tick <= latest_tick_; ++tick) {
        if (auto input = get(tick)) {
            result.push_back(*input);
        }
    }
    return result;
}

} // namespace city
