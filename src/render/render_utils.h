#pragma once

#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>
#include <cassert>

inline void vk_assert_msg(VkResult result, const char* message) {
    if (result != VK_SUCCESS) {
        spdlog::critical("Vulkan assertion failed: {}", message);
        assert(false);
    }
}

inline void vk_assert_msg(VkResult result, std::string message) {
    if (result != VK_SUCCESS) {
        spdlog::critical("Vulkan assertion failed: {}", message);
        assert(false);
    }
}