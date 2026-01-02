#include "profiler.hpp"

#ifdef __linux__
#include <fstream>
#include <unistd.h>
#endif

namespace city {

void TickProfiler::begin_tick(u32 tick_number) {
    current_tick_ = TickProfile{};
    current_tick_.tick_number = tick_number;

    // Transfer accumulated phase times (e.g., network time since last tick)
    for (size_t i = 0; i < TICK_PHASE_COUNT; ++i) {
        current_tick_.phase_times_us[i] = accumulated_phase_times_us_[i];
        accumulated_phase_times_us_[i] = 0.0;
    }

    tick_start_ = Clock::now();
    in_tick_ = true;
}

void TickProfiler::end_tick() {
    if (!in_tick_) return;

    // Calculate total time as sum of all phase times
    // This includes accumulated network time from before begin_tick
    current_tick_.total_time_us = 0.0;
    for (size_t i = 0; i < TICK_PHASE_COUNT; ++i) {
        current_tick_.total_time_us += current_tick_.phase_times_us[i];
    }

    // Update memory usage
    update_memory_usage();

    // Record spike if exceeded budget
    if (current_tick_.exceeded_budget()) {
        SpikeRecord spike;
        spike.tick_number = current_tick_.tick_number;
        spike.total_time_ms = current_tick_.total_time_ms();

        // Find worst phase
        f64 max_phase_time = 0.0;
        spike.worst_phase = TickPhase::Network;
        for (size_t i = 0; i < TICK_PHASE_COUNT; ++i) {
            if (current_tick_.phase_times_us[i] > max_phase_time) {
                max_phase_time = current_tick_.phase_times_us[i];
                spike.worst_phase = static_cast<TickPhase>(i);
            }
        }
        spike.worst_phase_time_ms = max_phase_time / 1000.0;

        spikes_.push(spike);
    }

    // Store in history
    history_.push(current_tick_);
    in_tick_ = false;
}

void TickProfiler::begin_phase(TickPhase phase) {
    if (in_phase_) {
        end_phase();  // Auto-end previous phase
    }
    current_phase_ = phase;
    phase_start_ = Clock::now();
    in_phase_ = true;
}

void TickProfiler::end_phase() {
    if (!in_phase_) return;

    auto end_time = Clock::now();
    f64 duration_us = std::chrono::duration<f64, std::micro>(end_time - phase_start_).count();

    if (in_tick_) {
        // Add to current tick
        current_tick_.phase_times_us[static_cast<size_t>(current_phase_)] += duration_us;
    } else {
        // Accumulate for next tick (e.g., network time between simulation ticks)
        accumulated_phase_times_us_[static_cast<size_t>(current_phase_)] += duration_us;
    }

    in_phase_ = false;
}

void TickProfiler::begin_scope(std::string_view name) {
    std::string name_str(name);
    auto& entry = scopes_[name_str];
    entry.start = Clock::now();
    current_scope_ = name_str;
}

void TickProfiler::end_scope(std::string_view name) {
    std::string name_str(name);
    auto it = scopes_.find(name_str);
    if (it != scopes_.end()) {
        auto end_time = Clock::now();
        f64 duration_us = std::chrono::duration<f64, std::micro>(end_time - it->second.start).count();
        it->second.accumulated_us += duration_us;
        it->second.max_us = std::max(it->second.max_us, duration_us);
        it->second.call_count++;
    }
    current_scope_.clear();
}

const TickProfile& TickProfiler::latest() const {
    if (history_.empty()) {
        return current_tick_;
    }
    return history_.back();
}

f64 TickProfiler::average_tick_time_ms(size_t sample_count) const {
    if (history_.empty()) return 0.0;

    size_t count = std::min(sample_count, history_.size());
    f64 total = 0.0;
    for (size_t i = history_.size() - count; i < history_.size(); ++i) {
        total += history_[i].total_time_us;
    }
    return (total / static_cast<f64>(count)) / 1000.0;
}

f64 TickProfiler::max_tick_time_ms(size_t sample_count) const {
    if (history_.empty()) return 0.0;

    size_t count = std::min(sample_count, history_.size());
    f64 max_time = 0.0;
    for (size_t i = history_.size() - count; i < history_.size(); ++i) {
        max_time = std::max(max_time, history_[i].total_time_us);
    }
    return max_time / 1000.0;
}

f64 TickProfiler::min_tick_time_ms(size_t sample_count) const {
    if (history_.empty()) return 0.0;

    size_t count = std::min(sample_count, history_.size());
    f64 min_time = history_[history_.size() - count].total_time_us;
    for (size_t i = history_.size() - count; i < history_.size(); ++i) {
        min_time = std::min(min_time, history_[i].total_time_us);
    }
    return min_time / 1000.0;
}

u32 TickProfiler::spike_count(size_t sample_count) const {
    if (history_.empty()) return 0;

    size_t count = std::min(sample_count, history_.size());
    u32 spikes = 0;
    for (size_t i = history_.size() - count; i < history_.size(); ++i) {
        if (history_[i].exceeded_budget()) {
            ++spikes;
        }
    }
    return spikes;
}

f64 TickProfiler::average_phase_time_ms(TickPhase phase, size_t sample_count) const {
    if (history_.empty()) return 0.0;

    size_t count = std::min(sample_count, history_.size());
    size_t phase_idx = static_cast<size_t>(phase);
    f64 total = 0.0;
    for (size_t i = history_.size() - count; i < history_.size(); ++i) {
        total += history_[i].phase_times_us[phase_idx];
    }
    return (total / static_cast<f64>(count)) / 1000.0;
}

std::vector<TickProfiler::ScopeStats> TickProfiler::get_scope_stats() const {
    std::vector<ScopeStats> stats;
    stats.reserve(scopes_.size());

    for (const auto& [name, entry] : scopes_) {
        ScopeStats s;
        s.name = name;
        s.total_time_us = entry.accumulated_us;
        s.call_count = entry.call_count;
        s.average_time_us = entry.call_count > 0 ? entry.accumulated_us / entry.call_count : 0.0;
        s.max_time_us = entry.max_us;
        stats.push_back(s);
    }

    // Sort by total time descending
    std::sort(stats.begin(), stats.end(), [](const ScopeStats& a, const ScopeStats& b) {
        return a.total_time_us > b.total_time_us;
    });

    return stats;
}

void TickProfiler::reset_scope_stats() {
    scopes_.clear();
}

void TickProfiler::update_memory_usage() {
#ifdef __linux__
    std::ifstream statm("/proc/self/statm");
    if (statm) {
        size_t size, resident;
        statm >> size >> resident;
        // resident is in pages, multiply by page size
        last_memory_usage_ = resident * static_cast<size_t>(sysconf(_SC_PAGESIZE));
    }
#endif
    current_tick_.memory_usage_bytes = last_memory_usage_;
}

} // namespace city
