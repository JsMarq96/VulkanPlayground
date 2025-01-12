#pragma once

#include <vulkan/vulkan.h>

void vk_assert_msg(VkResult result, const char* message);

//void vk_assert_msg(VkResult result, std::string message);