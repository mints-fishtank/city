#include "vk_context.hpp"
#include <SDL3/SDL.h>
#include <iostream>

namespace city {

VkContext::~VkContext() {
    shutdown();
}

bool VkContext::init(SDL_Window* /*window*/) {
    // TODO: Initialize Vulkan
    // 1. Create VkInstance
    // 2. Create surface from SDL window
    // 3. Select physical device
    // 4. Create logical device
    // 5. Create swapchain
    // 6. Create render pass
    // 7. Create framebuffers
    // 8. Create command pool/buffers

    std::cout << "Vulkan context init (stub)\n";
    valid_ = true;
    return true;
}

void VkContext::shutdown() {
    if (!valid_) return;

    // TODO: Cleanup Vulkan resources
    valid_ = false;
}

void VkContext::begin_frame() {
    // TODO: Acquire next swapchain image
    // TODO: Begin command buffer
}

void VkContext::end_frame() {
    // TODO: End command buffer
    // TODO: Submit to queue
    // TODO: Present swapchain image
}

} // namespace city
