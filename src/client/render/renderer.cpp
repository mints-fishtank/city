#include "renderer.hpp"
#include "core/game/components/transform.hpp"
#include "core/game/components/player.hpp"
#include <SDL3/SDL.h>
#include <iostream>

namespace city {

Renderer::Renderer() = default;

Renderer::~Renderer() {
    shutdown();
}

bool Renderer::init(u32 width, u32 height, const std::string& title) {
    width_ = width;
    height_ = height;

    // Create SDL window
    window_ = SDL_CreateWindow(
        title.c_str(),
        static_cast<int>(width),
        static_cast<int>(height),
        SDL_WINDOW_RESIZABLE
    );

    if (!window_) {
        std::cerr << "Failed to create window: " << SDL_GetError() << "\n";
        return false;
    }

    // Create SDL renderer (GPU accelerated)
    sdl_renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!sdl_renderer_) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << "\n";
        return false;
    }

    // Enable vsync
    SDL_SetRenderVSync(sdl_renderer_, 1);

    std::cout << "Renderer initialized (SDL2D)\n";
    return true;
}

void Renderer::shutdown() {
    if (sdl_renderer_) {
        SDL_DestroyRenderer(sdl_renderer_);
        sdl_renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
}

void Renderer::begin_frame() {
    // Clear with dark background
    SDL_SetRenderDrawColor(sdl_renderer_, 20, 20, 30, 255);
    SDL_RenderClear(sdl_renderer_);
}

void Renderer::end_frame() {
    SDL_RenderPresent(sdl_renderer_);
}

Vec2f Renderer::world_to_screen(Vec2f world_pos) const {
    // Center camera on screen
    f32 screen_center_x = static_cast<f32>(width_) / 2.0f;
    f32 screen_center_y = static_cast<f32>(height_) / 2.0f;

    // World position relative to camera, scaled by zoom
    f32 rel_x = (world_pos.x - camera_pos_.x) * TILE_SIZE * camera_zoom_;
    f32 rel_y = (world_pos.y - camera_pos_.y) * TILE_SIZE * camera_zoom_;

    return {
        screen_center_x + rel_x,
        screen_center_y + rel_y
    };
}

Vec2f Renderer::screen_to_world(Vec2f screen_pos) const {
    f32 screen_center_x = static_cast<f32>(width_) / 2.0f;
    f32 screen_center_y = static_cast<f32>(height_) / 2.0f;

    f32 rel_x = (screen_pos.x - screen_center_x) / (TILE_SIZE * camera_zoom_);
    f32 rel_y = (screen_pos.y - screen_center_y) / (TILE_SIZE * camera_zoom_);

    return {
        camera_pos_.x + rel_x,
        camera_pos_.y + rel_y
    };
}

void Renderer::draw_rect(Rectf rect, Color color, bool filled) {
    SDL_SetRenderDrawColor(sdl_renderer_, color.r, color.g, color.b, color.a);

    SDL_FRect sdl_rect = {rect.x, rect.y, rect.width, rect.height};
    if (filled) {
        SDL_RenderFillRect(sdl_renderer_, &sdl_rect);
    } else {
        SDL_RenderRect(sdl_renderer_, &sdl_rect);
    }
}

void Renderer::draw_rect_world(Rectf rect, Color color, bool filled) {
    Vec2f screen_pos = world_to_screen({rect.x, rect.y});
    f32 scaled_w = rect.width * TILE_SIZE * camera_zoom_;
    f32 scaled_h = rect.height * TILE_SIZE * camera_zoom_;

    draw_rect({screen_pos.x, screen_pos.y, scaled_w, scaled_h}, color, filled);
}

void Renderer::render_tilemap(const TileMap& tilemap) {
    // Calculate visible tile range
    Vec2f top_left = screen_to_world({0, 0});
    Vec2f bottom_right = screen_to_world({static_cast<f32>(width_), static_cast<f32>(height_)});

    i32 start_x = static_cast<i32>(std::floor(top_left.x)) - 1;
    i32 start_y = static_cast<i32>(std::floor(top_left.y)) - 1;
    i32 end_x = static_cast<i32>(std::ceil(bottom_right.x)) + 1;
    i32 end_y = static_cast<i32>(std::ceil(bottom_right.y)) + 1;

    // Clamp to tilemap bounds
    start_x = std::max(start_x, 0);
    start_y = std::max(start_y, 0);
    end_x = std::min(end_x, static_cast<i32>(tilemap.width()));
    end_y = std::min(end_y, static_cast<i32>(tilemap.height()));

    // Render tiles
    for (i32 y = start_y; y < end_y; ++y) {
        for (i32 x = start_x; x < end_x; ++x) {
            const Tile* tile = tilemap.get_tile({x, y});
            if (!tile) continue;

            // Simple color based on tile type
            Color floor_color;
            if (tile->floor_id == 0) {
                floor_color = {40, 40, 50, 255};  // Empty/void
            } else {
                floor_color = {60, 90, 60, 255};  // Grass/floor
            }

            draw_rect_world({static_cast<f32>(x), static_cast<f32>(y), 1.0f, 1.0f}, floor_color, true);

            // Draw walls
            if (tile->has_wall()) {
                Color wall_color = {100, 80, 60, 255};  // Brown wall
                draw_rect_world({static_cast<f32>(x), static_cast<f32>(y), 1.0f, 1.0f}, wall_color, true);
            }

            // Draw grid lines (subtle)
            Color grid_color = {50, 50, 60, 255};
            draw_rect_world({static_cast<f32>(x), static_cast<f32>(y), 1.0f, 1.0f}, grid_color, false);
        }
    }
}

void Renderer::render_entities(World& world) {
    // Render all entities with Transform and Player components
    world.each<Transform, Player>([this](Entity /*e*/, Transform& transform, Player& player) {
        // Draw player as a colored rectangle
        Color player_color = player.is_local ? Color{100, 150, 255, 255} : Color{255, 150, 100, 255};

        // Player size slightly smaller than a tile
        f32 size = 0.8f;
        // Center the player on their position (position is tile center)
        f32 half_size = size / 2.0f;

        draw_rect_world({
            transform.position.x - half_size,
            transform.position.y - half_size,
            size,
            size
        }, player_color, true);

        // Draw outline
        Color outline_color = {255, 255, 255, 200};
        draw_rect_world({
            transform.position.x - half_size,
            transform.position.y - half_size,
            size,
            size
        }, outline_color, false);
    });
}

void Renderer::set_camera_position(Vec2f position) {
    camera_pos_ = position;
}

void Renderer::set_camera_zoom(f32 zoom) {
    camera_zoom_ = std::clamp(zoom, 0.25f, 4.0f);
}

} // namespace city
