#pragma once

#include "core/game/components/player.hpp"
#include <array>
#include <optional>
#include <vector>

namespace city {

// Circular buffer for storing inputs for resimulation
class InputBuffer {
public:
    static constexpr size_t BUFFER_SIZE = 128;

    void add(InputSnapshot input);

    // Clear all inputs (used when re-syncing ticks)
    void clear();

    // Mark a tick as acknowledged (we keep inputs for resimulation)
    void acknowledge(u32 tick);

    // Get input for a specific tick
    std::optional<InputSnapshot> get(u32 tick) const;

    // Get all inputs after a specific tick (for resimulation)
    std::vector<InputSnapshot> get_inputs_after(u32 tick) const;

    // Get unacknowledged inputs (inputs after last_acked_tick)
    std::vector<InputSnapshot> get_unacknowledged() const;

    u32 last_acked_tick() const { return last_acked_tick_; }
    u32 latest_tick() const { return latest_tick_; }
    u32 oldest_tick() const { return oldest_tick_; }

private:
    std::array<std::optional<InputSnapshot>, BUFFER_SIZE> buffer_;
    u32 last_acked_tick_{0};
    u32 latest_tick_{0};
    u32 oldest_tick_{0};  // Oldest tick still in buffer
};

} // namespace city
