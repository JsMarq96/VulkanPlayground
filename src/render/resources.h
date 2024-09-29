#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

struct sImage {
    VkImage         image;
    VkImageView     image_view;
    VmaAllocation   alloc;
    VkExtent3D      extent;
    VkFormat        format;
};