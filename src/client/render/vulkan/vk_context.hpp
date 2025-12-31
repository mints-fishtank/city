#pragma once

#include "core/util/types.hpp"

namespace city {

// Vulkan context - manages Vulkan instance, device, swapchain
class VkContext {
public:
    VkContext() = default;
    ~VkContext();

    bool init(struct SDL_Window* window);
    void shutdown();

    void begin_frame();
    void end_frame();

    bool is_valid() const { return valid_; }

private:
    bool valid_{false};
    // Vulkan handles will be added here
};

} // namespace city
