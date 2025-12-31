#include "renderer.hpp"
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

    // Create SDL window with Vulkan support
    window_ = SDL_CreateWindow(
        title.c_str(),
        static_cast<int>(width),
        static_cast<int>(height),
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );

    if (!window_) {
        std::cerr << "Failed to create window: " << SDL_GetError() << "\n";
        return false;
    }

    // TODO: Initialize Vulkan
    std::cout << "Renderer initialized (Vulkan setup pending)\n";
    return true;
}

void Renderer::shutdown() {
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
}

void Renderer::begin_frame() {
    // TODO: Begin Vulkan frame
}

void Renderer::end_frame() {
    // TODO: End Vulkan frame and present
}

void Renderer::render_tilemap(const TileMap& /*tilemap*/) {
    // TODO: Render tiles
}

void Renderer::render_entities(World& /*world*/) {
    // TODO: Render entity sprites
}

void Renderer::set_camera_position(Vec2f position) {
    camera_pos_ = position;
}

void Renderer::set_camera_zoom(f32 zoom) {
    camera_zoom_ = zoom;
}

} // namespace city
