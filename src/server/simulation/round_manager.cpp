#include "round_manager.hpp"
#include "../server.hpp"

namespace city {

RoundManager::RoundManager(Server& server) : server_(server) {
    (void)server_;  // Will be used for broadcasting round events
}

void RoundManager::update(f32 dt) {
    state_timer_ += dt;

    switch (state_) {
        case RoundState::Lobby:
            // Wait for enough players or admin command
            break;

        case RoundState::Starting:
            if (state_timer_ >= 10.0f) {  // 10 second countdown
                state_ = RoundState::Playing;
                state_timer_ = 0.0f;
                time_remaining_ = round_duration_;
            }
            break;

        case RoundState::Playing:
            time_remaining_ -= dt;
            if (time_remaining_ <= 0.0f) {
                end_round();
            }
            break;

        case RoundState::Ending:
            if (state_timer_ >= 30.0f) {  // 30 second results screen
                state_ = RoundState::Lobby;
                state_timer_ = 0.0f;
            }
            break;
    }
}

void RoundManager::start_round() {
    if (state_ != RoundState::Lobby) return;
    state_ = RoundState::Starting;
    state_timer_ = 0.0f;
}

void RoundManager::end_round() {
    state_ = RoundState::Ending;
    state_timer_ = 0.0f;
    time_remaining_ = 0.0f;
}

} // namespace city
