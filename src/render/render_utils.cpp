#include "render_utils.h"

#include <cassert>
#include <spdlog/spdlog.h>

void vk_assert_msg(VkResult result, const char* message) {
    if (result != VK_SUCCESS) {
        spdlog::critical("Vulkan assertion failed: {}", message);
        assert(false);
    }
}

void vk_assert_msg(VkResult result, std::string message) {
    if (result != VK_SUCCESS) {
        spdlog::critical("Vulkan assertion failed: {}", message);
        assert(false);
    }
}