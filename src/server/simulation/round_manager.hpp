#pragma once

#include "core/util/types.hpp"

namespace city {

class Server;

enum class RoundState {
    Lobby,      // Waiting for players
    Starting,   // Countdown to round start
    Playing,    // Round in progress
    Ending,     // Round ending, showing results
};

class RoundManager {
public:
    explicit RoundManager(Server& server);

    void update(f32 dt);

    void start_round();
    void end_round();

    RoundState state() const { return state_; }
    f32 time_remaining() const { return time_remaining_; }
    f32 round_duration() const { return round_duration_; }

    void set_round_duration(f32 seconds) { round_duration_ = seconds; }

private:
    Server& server_;
    RoundState state_{RoundState::Lobby};
    f32 round_duration_{180.0f * 60.0f};  // 3 hours in seconds
    f32 time_remaining_{0.0f};
    f32 state_timer_{0.0f};
};

} // namespace city
