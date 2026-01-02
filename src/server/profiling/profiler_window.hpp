#pragma once

#include "profiler.hpp"

struct SDL_Window;
struct SDL_Renderer;

namespace city {

class ProfilerWindow {
public:
    ProfilerWindow();
    ~ProfilerWindow();

    // Initialize SDL window and ImGUI context
    bool init(u32 width = 900, u32 height = 700, const char* title = "Server Profiler");
    void shutdown();

    // Call each frame to process events and render
    // Returns false if window was closed
    bool update();

    // Set the profiler to visualize
    void set_profiler(TickProfiler* profiler) { profiler_ = profiler; }

    bool is_open() const { return window_ != nullptr; }

private:
    void render();
    void render_tick_overview();
    void render_phase_breakdown();
    void render_phase_timeline();  // Stacked area graph of phases over time
    void render_tick_graph();
    void render_spike_list();
    void render_scope_timing();
    void render_entity_stats();
    void render_memory_usage();

    SDL_Window* window_{nullptr};
    SDL_Renderer* renderer_{nullptr};
    TickProfiler* profiler_{nullptr};

    // UI state
    bool paused_{false};
    int graph_time_range_{300};  // Frames to show in graph (5 seconds default)
    int stats_time_range_{60};   // Sample count for statistics (1 second default)

    // Graph data cache
    std::vector<float> tick_time_cache_;
    std::vector<float> phase_time_cache_[TICK_PHASE_COUNT];
};

} // namespace city
