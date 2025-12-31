#pragma once

#include "core/util/types.hpp"
#include "core/ecs/world.hpp"
#include "core/grid/tilemap.hpp"
#include <string>

namespace city {

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init(u32 width, u32 height, const std::string& title);
    void shutdown();

    void begin_frame();
    void end_frame();

    void render_tilemap(const TileMap& tilemap);
    void render_entities(World& world);

    // Camera controls
    void set_camera_position(Vec2f position);
    void set_camera_zoom(f32 zoom);
    Vec2f camera_position() const { return camera_pos_; }
    f32 camera_zoom() const { return camera_zoom_; }

    // Window info
    u32 width() const { return width_; }
    u32 height() const { return height_; }

private:
    u32 width_{0};
    u32 height_{0};
    Vec2f camera_pos_{0.0f, 0.0f};
    f32 camera_zoom_{1.0f};

    // SDL/Vulkan handles (to be implemented)
    struct SDL_Window* window_{nullptr};
};

} // namespace city
