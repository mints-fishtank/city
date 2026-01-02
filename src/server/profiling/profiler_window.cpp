#include "profiler_window.hpp"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <algorithm>
#include <cstdio>

namespace city {

// Phase colors for consistent visualization
static const ImVec4 PHASE_COLORS[TICK_PHASE_COUNT] = {
    ImVec4(0.2f, 0.6f, 1.0f, 1.0f),   // Network - blue
    ImVec4(0.4f, 0.8f, 0.2f, 1.0f),   // Input - green
    ImVec4(1.0f, 0.6f, 0.2f, 1.0f),   // World - orange
    ImVec4(0.8f, 0.2f, 0.8f, 1.0f),   // Round - purple
    ImVec4(0.9f, 0.9f, 0.2f, 1.0f),   // Broadcast - yellow
};

ProfilerWindow::ProfilerWindow() = default;

ProfilerWindow::~ProfilerWindow() {
    shutdown();
}

bool ProfilerWindow::init(u32 width, u32 height, const char* title) {
    // Create SDL window
    window_ = SDL_CreateWindow(
        title,
        static_cast<int>(width),
        static_cast<int>(height),
        SDL_WINDOW_RESIZABLE
    );

    if (!window_) {
        SDL_Log("Failed to create profiler window: %s", SDL_GetError());
        return false;
    }

    // Create SDL renderer
    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!renderer_) {
        SDL_Log("Failed to create profiler renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        return false;
    }

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup ImGui style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 4.0f;
    style.FrameRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window_, renderer_);
    ImGui_ImplSDLRenderer3_Init(renderer_);

    // Pre-allocate graph data
    tick_time_cache_.reserve(600);
    for (auto& cache : phase_time_cache_) {
        cache.reserve(600);
    }

    return true;
}

void ProfilerWindow::shutdown() {
    if (renderer_) {
        ImGui_ImplSDLRenderer3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }

    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
}

bool ProfilerWindow::update() {
    if (!window_) return false;

    // Process events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);

        if (event.type == SDL_EVENT_QUIT) {
            shutdown();
            return false;
        }
        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
            event.window.windowID == SDL_GetWindowID(window_)) {
            shutdown();
            return false;
        }
    }

    // Start ImGui frame
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // Render profiler UI
    render();

    // Render ImGui
    ImGui::Render();
    SDL_SetRenderDrawColor(renderer_, 30, 30, 30, 255);
    SDL_RenderClear(renderer_);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer_);
    SDL_RenderPresent(renderer_);

    return true;
}

void ProfilerWindow::render() {
    if (!profiler_) {
        ImGui::Begin("Server Profiler");
        ImGui::Text("No profiler attached");
        ImGui::End();
        return;
    }

    // Get window size for full-window layout
    int win_w, win_h;
    SDL_GetWindowSize(window_, &win_w, &win_h);

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(win_w), static_cast<float>(win_h)));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("Server Profiler", nullptr, flags);

    // Header with controls
    ImGui::Text("Server Tick Profiler");
    ImGui::SameLine(ImGui::GetWindowWidth() - 200);
    ImGui::Checkbox("Pause", &paused_);
    ImGui::SameLine();
    if (ImGui::Button("Reset Stats")) {
        profiler_->reset_scope_stats();
    }

    ImGui::Separator();

    render_tick_overview();
    render_phase_breakdown();
    render_phase_timeline();
    render_tick_graph();
    render_spike_list();
    render_scope_timing();
    render_entity_stats();
    render_memory_usage();

    ImGui::End();
}

void ProfilerWindow::render_tick_overview() {
    if (!ImGui::CollapsingHeader("Tick Overview", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }

    const auto& tick = paused_ ? profiler_->latest() : profiler_->current();

    // Current tick info
    ImGui::Text("Tick: %u", tick.tick_number);
    ImGui::SameLine(150);

    // Tick time with progress bar
    float progress = static_cast<float>(tick.total_time_ms() / TickProfiler::TARGET_TICK_TIME_MS);
    progress = std::min(progress, 2.0f);  // Cap at 200%

    ImVec4 bar_color = tick.exceeded_budget()
        ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f)   // Red if over budget
        : ImVec4(0.3f, 0.8f, 0.3f, 1.0f);  // Green if ok

    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, bar_color);
    char overlay[64];
    snprintf(overlay, sizeof(overlay), "%.2f / %.2f ms", tick.total_time_ms(), TickProfiler::TARGET_TICK_TIME_MS);
    ImGui::ProgressBar(progress / 2.0f, ImVec2(250, 0), overlay);  // /2 because we capped at 200%
    ImGui::PopStyleColor();

    // Statistics
    ImGui::SameLine(450);
    ImGui::Text("Stats period:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::SliderInt("##stats_range", &stats_time_range_, 30, 300, "%d frames");

    size_t sample_count = static_cast<size_t>(stats_time_range_);
    ImGui::Text("Avg: %.2f ms  |  Min: %.2f ms  |  Max: %.2f ms  |  Spikes: %u",
        profiler_->average_tick_time_ms(sample_count),
        profiler_->min_tick_time_ms(sample_count),
        profiler_->max_tick_time_ms(sample_count),
        profiler_->spike_count(sample_count));

    ImGui::Spacing();
}

void ProfilerWindow::render_phase_breakdown() {
    if (!ImGui::CollapsingHeader("Phase Breakdown", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }

    const auto& tick = paused_ ? profiler_->latest() : profiler_->current();

    // Stacked horizontal bar
    float total_width = ImGui::GetContentRegionAvail().x - 20;
    ImVec2 bar_start = ImGui::GetCursorScreenPos();
    float x_offset = 0;

    // Draw phase bars
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    float bar_height = 24;

    if (tick.total_time_us > 0) {
        for (size_t i = 0; i < TICK_PHASE_COUNT; ++i) {
            float width = total_width * static_cast<float>(tick.phase_times_us[i] / tick.total_time_us);
            if (width > 0) {
                draw_list->AddRectFilled(
                    ImVec2(bar_start.x + x_offset, bar_start.y),
                    ImVec2(bar_start.x + x_offset + width, bar_start.y + bar_height),
                    ImGui::ColorConvertFloat4ToU32(PHASE_COLORS[i]));
                x_offset += width;
            }
        }
    }

    // Draw budget line overlay
    float budget_x = total_width * static_cast<float>(TickProfiler::TARGET_TICK_TIME_MS / 25.0);  // 25ms max display
    if (budget_x < total_width) {
        draw_list->AddLine(
            ImVec2(bar_start.x + budget_x, bar_start.y),
            ImVec2(bar_start.x + budget_x, bar_start.y + bar_height),
            IM_COL32(255, 100, 100, 200), 2.0f);
    }

    ImGui::Dummy(ImVec2(total_width, bar_height + 5));

    // Legend with values
    ImGui::Columns(3, "phase_legend", false);
    for (size_t i = 0; i < TICK_PHASE_COUNT; ++i) {
        ImGui::ColorButton("##color", PHASE_COLORS[i], ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder, ImVec2(12, 12));
        ImGui::SameLine();

        f64 pct = tick.total_time_us > 0 ? (tick.phase_times_us[i] / tick.total_time_us) * 100.0 : 0.0;
        ImGui::Text("%s: %.2fms (%.1f%%)",
            PHASE_NAMES[i],
            tick.phase_times_us[i] / 1000.0,
            pct);

        if ((i + 1) % 2 == 0) {
            ImGui::NextColumn();
        }
    }
    ImGui::Columns(1);

    ImGui::Spacing();
}

void ProfilerWindow::render_phase_timeline() {
    if (!ImGui::CollapsingHeader("Phase Timeline (Stacked)", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }

    const auto& history = profiler_->history();
    size_t count = std::min(static_cast<size_t>(graph_time_range_), history.size());

    if (count == 0) {
        ImGui::Text("No data yet...");
        ImGui::Spacing();
        return;
    }

    // Time range selector (shared with tick graph)
    ImGui::Text("Time range:");
    ImGui::SameLine();
    if (ImGui::Button("1s##timeline")) graph_time_range_ = 60;
    ImGui::SameLine();
    if (ImGui::Button("5s##timeline")) graph_time_range_ = 300;
    ImGui::SameLine();
    if (ImGui::Button("10s##timeline")) graph_time_range_ = 600;

    // Graph dimensions
    float graph_width = ImGui::GetContentRegionAvail().x;
    float graph_height = 180;
    ImVec2 graph_pos = ImGui::GetCursorScreenPos();

    // Reserve space for the graph
    ImGui::Dummy(ImVec2(graph_width, graph_height));

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Background
    draw_list->AddRectFilled(
        graph_pos,
        ImVec2(graph_pos.x + graph_width, graph_pos.y + graph_height),
        IM_COL32(40, 40, 40, 255));

    // Calculate max time for scaling (auto-scale to actual data)
    float max_time_ms = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        float total = static_cast<float>(history[history.size() - count + i].total_time_ms());
        if (total > max_time_ms) max_time_ms = total;
    }
    // Add 20% headroom, with small minimum to avoid division issues
    max_time_ms = std::max(max_time_ms * 1.2f, 0.001f);
    // Round to nice values for readability
    if (max_time_ms < 0.1f) max_time_ms = std::ceil(max_time_ms * 100.0f) / 100.0f;
    else if (max_time_ms < 1.0f) max_time_ms = std::ceil(max_time_ms * 10.0f) / 10.0f;
    else if (max_time_ms < 10.0f) max_time_ms = std::ceil(max_time_ms);
    else max_time_ms = std::ceil(max_time_ms / 5.0f) * 5.0f;

    // Calculate how many ticks to aggregate per bar (for smoother rendering when zoomed out)
    float min_bar_width = 2.0f;  // Minimum pixels per bar
    size_t ticks_per_bar = 1;
    float bar_width = graph_width / static_cast<float>(count);
    if (bar_width < min_bar_width) {
        ticks_per_bar = static_cast<size_t>(std::ceil(min_bar_width / bar_width));
        bar_width = graph_width / std::ceil(static_cast<float>(count) / static_cast<float>(ticks_per_bar));
    }

    size_t num_bars = (count + ticks_per_bar - 1) / ticks_per_bar;

    // Calculate proportional gap between bars (10% of bar width, clamped to 1-3 pixels)
    float bar_gap = std::clamp(bar_width * 0.1f, 1.0f, 3.0f);
    float bar_inner_width = bar_width - bar_gap;

    for (size_t bar = 0; bar < num_bars; ++bar) {
        // Aggregate ticks for this bar
        std::array<f64, TICK_PHASE_COUNT> aggregated_phases{};
        size_t start_idx = bar * ticks_per_bar;
        size_t end_idx = std::min(start_idx + ticks_per_bar, count);
        size_t ticks_in_bar = end_idx - start_idx;

        for (size_t i = start_idx; i < end_idx; ++i) {
            const auto& tick = history[history.size() - count + i];
            for (size_t p = 0; p < TICK_PHASE_COUNT; ++p) {
                aggregated_phases[p] += tick.phase_times_us[p];
            }
        }

        // Average the aggregated times
        for (size_t p = 0; p < TICK_PHASE_COUNT; ++p) {
            aggregated_phases[p] /= static_cast<f64>(ticks_in_bar);
        }

        float x = graph_pos.x + static_cast<float>(bar) * bar_width;
        float y_bottom = graph_pos.y + graph_height;
        float y_accumulated = 0.0f;

        // Draw each phase stacked from bottom to top
        for (size_t p = 0; p < TICK_PHASE_COUNT; ++p) {
            float phase_ms = static_cast<float>(aggregated_phases[p] / 1000.0);
            float phase_height = (phase_ms / max_time_ms) * graph_height;

            if (phase_height > 0.5f) {  // Only draw if visible
                draw_list->AddRectFilled(
                    ImVec2(x, y_bottom - y_accumulated - phase_height),
                    ImVec2(x + bar_inner_width, y_bottom - y_accumulated),
                    ImGui::ColorConvertFloat4ToU32(PHASE_COLORS[p]));
            }

            y_accumulated += phase_height;
        }
    }

    // Draw budget line (only if visible in current scale)
    if (max_time_ms >= TickProfiler::TARGET_TICK_TIME_MS * 0.5f) {
        float budget_y = graph_pos.y + graph_height - (TickProfiler::TARGET_TICK_TIME_MS / max_time_ms) * graph_height;
        budget_y = std::max(budget_y, graph_pos.y);  // Clamp to graph area
        draw_list->AddLine(
            ImVec2(graph_pos.x, budget_y),
            ImVec2(graph_pos.x + graph_width, budget_y),
            IM_COL32(255, 100, 100, 200), 2.0f);

        // Budget label
        char budget_label[32];
        snprintf(budget_label, sizeof(budget_label), "%.1fms budget", TickProfiler::TARGET_TICK_TIME_MS);
        draw_list->AddText(
            ImVec2(graph_pos.x + graph_width - 90, budget_y + 2),
            IM_COL32(255, 100, 100, 255),
            budget_label);
    }

    // Y-axis labels (0ms at bottom, max at top)
    char max_label[32];
    if (max_time_ms < 1.0f) {
        snprintf(max_label, sizeof(max_label), "%.2fms", max_time_ms);
    } else {
        snprintf(max_label, sizeof(max_label), "%.1fms", max_time_ms);
    }
    draw_list->AddText(
        ImVec2(graph_pos.x + 5, graph_pos.y + 2),
        IM_COL32(200, 200, 200, 255),
        max_label);

    draw_list->AddText(
        ImVec2(graph_pos.x + 5, graph_pos.y + graph_height - 15),
        IM_COL32(200, 200, 200, 255),
        "0ms");

    // Legend
    ImGui::Spacing();
    for (size_t i = 0; i < TICK_PHASE_COUNT; ++i) {
        ImGui::ColorButton("##color", PHASE_COLORS[i], ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder, ImVec2(12, 12));
        ImGui::SameLine();
        ImGui::Text("%s", PHASE_NAMES[i]);
        if (i < TICK_PHASE_COUNT - 1) {
            ImGui::SameLine();
            ImGui::Text("  |  ");
            ImGui::SameLine();
        }
    }

    ImGui::Spacing();
}

void ProfilerWindow::render_tick_graph() {
    if (!ImGui::CollapsingHeader("Tick Time Graph", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }

    // Time range selector
    ImGui::Text("Time range:");
    ImGui::SameLine();
    if (ImGui::Button("1s")) graph_time_range_ = 60;
    ImGui::SameLine();
    if (ImGui::Button("5s")) graph_time_range_ = 300;
    ImGui::SameLine();
    if (ImGui::Button("10s")) graph_time_range_ = 600;
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120);
    ImGui::SliderInt("##graph_range", &graph_time_range_, 30, 600, "%d frames");

    const auto& history = profiler_->history();
    size_t count = std::min(static_cast<size_t>(graph_time_range_), history.size());

    if (count == 0) {
        ImGui::Text("No data yet...");
        ImGui::Spacing();
        return;
    }

    // Build plot data
    tick_time_cache_.resize(count);
    for (size_t i = 0; i < count; ++i) {
        tick_time_cache_[i] = static_cast<float>(history[history.size() - count + i].total_time_ms());
    }

    // Draw the plot
    float graph_height = 120;
    ImGui::PlotLines("##tickgraph", tick_time_cache_.data(), static_cast<int>(count),
        0, nullptr, 0.0f, 30.0f, ImVec2(ImGui::GetContentRegionAvail().x, graph_height));

    // Draw budget line overlay on the plot
    ImVec2 p0 = ImGui::GetItemRectMin();
    ImVec2 p1 = ImGui::GetItemRectMax();
    float budget_y = p0.y + (p1.y - p0.y) * (1.0f - static_cast<float>(TickProfiler::TARGET_TICK_TIME_MS) / 30.0f);
    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(p0.x, budget_y), ImVec2(p1.x, budget_y),
        IM_COL32(255, 100, 100, 180), 1.5f);

    // Label
    ImGui::GetWindowDrawList()->AddText(
        ImVec2(p1.x - 80, budget_y - 15),
        IM_COL32(255, 100, 100, 255),
        "16.67ms budget");

    ImGui::Spacing();
}

void ProfilerWindow::render_spike_list() {
    if (!ImGui::CollapsingHeader("Spike Detection (>16.67ms)")) {
        return;
    }

    const auto& spikes = profiler_->spikes();

    if (spikes.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), "No spikes detected");
        ImGui::Spacing();
        return;
    }

    // Show most recent spikes first
    ImGui::BeginChild("spike_list", ImVec2(0, 120), ImGuiChildFlags_Border);
    for (size_t i = spikes.size(); i > 0; --i) {
        const auto& spike = spikes[i - 1];
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.6f, 1.0f),
            "Tick %u: %.2fms - %s spike (%.2fms)",
            spike.tick_number,
            spike.total_time_ms,
            PHASE_NAMES[static_cast<size_t>(spike.worst_phase)],
            spike.worst_phase_time_ms);
    }
    ImGui::EndChild();

    ImGui::Spacing();
}

void ProfilerWindow::render_scope_timing() {
    if (!ImGui::CollapsingHeader("Detailed Scope Timing")) {
        return;
    }

    auto scope_stats = profiler_->get_scope_stats();

    if (scope_stats.empty()) {
        ImGui::Text("No detailed scope data (use PROFILE_SCOPE macro)");
        ImGui::Spacing();
        return;
    }

    ImGui::BeginChild("scope_list", ImVec2(0, 150), ImGuiChildFlags_Border);
    ImGui::Columns(5, "scope_columns");
    ImGui::SetColumnWidth(0, 180);
    ImGui::SetColumnWidth(1, 90);
    ImGui::SetColumnWidth(2, 90);
    ImGui::SetColumnWidth(3, 90);
    ImGui::SetColumnWidth(4, 70);

    ImGui::Text("Scope"); ImGui::NextColumn();
    ImGui::Text("Total"); ImGui::NextColumn();
    ImGui::Text("Avg"); ImGui::NextColumn();
    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Max"); ImGui::NextColumn();
    ImGui::Text("Calls"); ImGui::NextColumn();
    ImGui::Separator();

    for (const auto& stat : scope_stats) {
        ImGui::Text("%s", stat.name.c_str()); ImGui::NextColumn();
        ImGui::Text("%.2fms", stat.total_time_us / 1000.0); ImGui::NextColumn();
        ImGui::Text("%.3fms", stat.average_time_us / 1000.0); ImGui::NextColumn();
        // Highlight max if it's significantly higher than average (potential spike)
        bool is_spike = stat.max_time_us > stat.average_time_us * 10.0;
        if (is_spike) {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%.2fms", stat.max_time_us / 1000.0);
        } else {
            ImGui::Text("%.2fms", stat.max_time_us / 1000.0);
        }
        ImGui::NextColumn();
        ImGui::Text("%u", stat.call_count); ImGui::NextColumn();
    }

    ImGui::Columns(1);
    ImGui::EndChild();

    ImGui::Spacing();
}

void ProfilerWindow::render_entity_stats() {
    if (!ImGui::CollapsingHeader("Entity Statistics")) {
        return;
    }

    const auto& tick = paused_ ? profiler_->latest() : profiler_->current();

    ImGui::Columns(4, "entity_stats", false);
    ImGui::Text("Entities: %u", tick.entity_count);
    ImGui::NextColumn();
    ImGui::Text("Players: %u", tick.player_count);
    ImGui::NextColumn();
    ImGui::Text("Msgs In: %u", tick.messages_received);
    ImGui::NextColumn();
    ImGui::Text("Msgs Out: %u", tick.messages_sent);
    ImGui::Columns(1);

    ImGui::Spacing();
}

void ProfilerWindow::render_memory_usage() {
    if (!ImGui::CollapsingHeader("Memory Usage")) {
        return;
    }

    const auto& tick = paused_ ? profiler_->latest() : profiler_->current();

    if (tick.memory_usage_bytes == 0) {
        ImGui::Text("Memory tracking not available on this platform");
    } else {
        float mb = static_cast<float>(tick.memory_usage_bytes) / (1024.0f * 1024.0f);
        ImGui::Text("Resident Memory: %.1f MB", mb);

        // Simple memory bar
        float max_mb = 512.0f;  // Assume 512MB max for visualization
        float progress = mb / max_mb;
        ImGui::ProgressBar(progress, ImVec2(300, 0));
    }

    ImGui::Spacing();
}

} // namespace city
