#pragma once

#include "core/game/components/player.hpp"
#include <array>
#include <optional>
#include <vector>

namespace city {

// Circular buffer for storing unacknowledged inputs
class InputBuffer {
public:
    static constexpr size_t BUFFER_SIZE = 128;

    void add(InputSnapshot input);
    void acknowledge(u32 tick);

    std::optional<InputSnapshot> get(u32 tick) const;
    std::vector<InputSnapshot> get_unacknowledged() const;

    u32 last_acked_tick() const { return last_acked_tick_; }
    u32 latest_tick() const { return latest_tick_; }

private:
    std::array<std::optional<InputSnapshot>, BUFFER_SIZE> buffer_;
    u32 last_acked_tick_{0};
    u32 latest_tick_{0};
};

} // namespace city
