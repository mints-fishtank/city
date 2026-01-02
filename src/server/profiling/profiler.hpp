#pragma once

#include "core/util/types.hpp"
#include <chrono>
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <unordered_map>

namespace city {

// Compile-time profiling toggle
#ifdef ENABLE_PROFILING
    #define PROFILE_SCOPE(profiler, name) city::ProfileScope _profile_scope_##__LINE__(profiler, name)
#else
    #define PROFILE_SCOPE(profiler, name) ((void)0)
#endif

// Known tick phases (for consistent ordering/coloring)
enum class TickPhase : u8 {
    Network,           // process_network()
    InputProcessing,   // InputProcessor::update()
    WorldUpdate,       // World::update()
    RoundManager,      // RoundManager::update()
    BroadcastState,    // broadcast_state() / EntitySync
    Count
};

constexpr size_t TICK_PHASE_COUNT = static_cast<size_t>(TickPhase::Count);

constexpr std::array<const char*, TICK_PHASE_COUNT> PHASE_NAMES = {
    "Network",
    "Input Processing",
    "World Update",
    "Round Manager",
    "Broadcast State"
};

// Per-tick snapshot of all timing data
struct TickProfile {
    u32 tick_number{0};
    f64 total_time_us{0.0};
    std::array<f64, TICK_PHASE_COUNT> phase_times_us{};

    // Extended metrics
    u32 entity_count{0};
    u32 player_count{0};
    u32 messages_received{0};
    u32 messages_sent{0};
    size_t memory_usage_bytes{0};

    bool exceeded_budget() const { return total_time_us > 16666.67; } // 16.67ms

    f64 total_time_ms() const { return total_time_us / 1000.0; }
    f64 phase_time_ms(TickPhase phase) const {
        return phase_times_us[static_cast<size_t>(phase)] / 1000.0;
    }
};

// Ring buffer for historical data
template<typename T, size_t N>
class RingBuffer {
public:
    void push(const T& value) {
        data_[write_pos_] = value;
        write_pos_ = (write_pos_ + 1) % N;
        if (count_ < N) ++count_;
    }

    void push(T&& value) {
        data_[write_pos_] = std::move(value);
        write_pos_ = (write_pos_ + 1) % N;
        if (count_ < N) ++count_;
    }

    // idx 0 = oldest, idx count-1 = newest
    const T& operator[](size_t idx) const {
        size_t actual = (write_pos_ + N - count_ + idx) % N;
        return data_[actual];
    }

    T& operator[](size_t idx) {
        size_t actual = (write_pos_ + N - count_ + idx) % N;
        return data_[actual];
    }

    // Get most recent element
    const T& back() const {
        return data_[(write_pos_ + N - 1) % N];
    }

    size_t size() const { return count_; }
    size_t capacity() const { return N; }
    bool empty() const { return count_ == 0; }

    void clear() {
        write_pos_ = 0;
        count_ = 0;
    }

private:
    std::array<T, N> data_{};
    size_t write_pos_ = 0;
    size_t count_ = 0;
};

// Spike record for displaying problematic ticks
struct SpikeRecord {
    u32 tick_number;
    f64 total_time_ms;
    TickPhase worst_phase;
    f64 worst_phase_time_ms;
};

// Main profiler class
class TickProfiler {
public:
    static constexpr size_t HISTORY_SIZE = 600;  // 10 seconds at 60Hz
    static constexpr size_t SPIKE_HISTORY_SIZE = 20;  // Keep last 20 spikes
    static constexpr f64 TARGET_TICK_TIME_US = 16666.67;  // 16.67ms
    static constexpr f64 TARGET_TICK_TIME_MS = 16.667;

    TickProfiler() = default;

    // --- Timing API ---
    void begin_tick(u32 tick_number);
    void end_tick();

    void begin_phase(TickPhase phase);
    void end_phase();

    // For detailed sub-phase timing (e.g., per-system)
    void begin_scope(std::string_view name);
    void end_scope(std::string_view name);

    // --- Metrics API ---
    void set_entity_count(u32 count) { current_tick_.entity_count = count; }
    void set_player_count(u32 count) { current_tick_.player_count = count; }
    void add_messages_received(u32 count = 1) { current_tick_.messages_received += count; }
    void add_messages_sent(u32 count = 1) { current_tick_.messages_sent += count; }

    // --- Query API ---
    const TickProfile& current() const { return current_tick_; }
    const TickProfile& latest() const;
    const RingBuffer<TickProfile, HISTORY_SIZE>& history() const { return history_; }
    const RingBuffer<SpikeRecord, SPIKE_HISTORY_SIZE>& spikes() const { return spikes_; }

    // Statistics over last N samples
    f64 average_tick_time_ms(size_t sample_count = 60) const;
    f64 max_tick_time_ms(size_t sample_count = 60) const;
    f64 min_tick_time_ms(size_t sample_count = 60) const;
    u32 spike_count(size_t sample_count = 60) const;

    // Per-phase statistics
    f64 average_phase_time_ms(TickPhase phase, size_t sample_count = 60) const;

    // Detailed scope timing
    struct ScopeStats {
        std::string name;
        f64 total_time_us{0.0};
        f64 average_time_us{0.0};
        u32 call_count{0};
    };
    std::vector<ScopeStats> get_scope_stats() const;
    void reset_scope_stats();

    // --- Memory tracking ---
    void update_memory_usage();

private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;

    TickProfile current_tick_;
    RingBuffer<TickProfile, HISTORY_SIZE> history_;
    RingBuffer<SpikeRecord, SPIKE_HISTORY_SIZE> spikes_;

    TimePoint tick_start_;
    TimePoint phase_start_;
    TickPhase current_phase_{};
    bool in_tick_{false};
    bool in_phase_{false};

    // Accumulated phase times between ticks (e.g., network time)
    std::array<f64, TICK_PHASE_COUNT> accumulated_phase_times_us_{};

    // Scope tracking for detailed profiling
    struct ScopeEntry {
        TimePoint start;
        f64 accumulated_us{0.0};
        u32 call_count{0};
    };
    std::unordered_map<std::string, ScopeEntry> scopes_;
    std::string current_scope_;

    // Memory tracking
    size_t last_memory_usage_{0};
};

// RAII scope timer for phases
class PhaseScope {
public:
    PhaseScope(TickProfiler& profiler, TickPhase phase)
        : profiler_(profiler) {
        profiler_.begin_phase(phase);
    }
    ~PhaseScope() { profiler_.end_phase(); }

    PhaseScope(const PhaseScope&) = delete;
    PhaseScope& operator=(const PhaseScope&) = delete;

private:
    TickProfiler& profiler_;
};

// RAII scope timer for named scopes
class ProfileScope {
public:
    ProfileScope(TickProfiler& profiler, std::string_view name)
        : profiler_(profiler), name_(name) {
        profiler_.begin_scope(name_);
    }
    ~ProfileScope() { profiler_.end_scope(name_); }

    ProfileScope(const ProfileScope&) = delete;
    ProfileScope& operator=(const ProfileScope&) = delete;

private:
    TickProfiler& profiler_;
    std::string_view name_;
};

} // namespace city
