#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace Render {
    struct sGPUBuffer {
        VkBuffer buffer;
        VmaAllocation alloc;
        VmaAllocationInfo alloc_info;
    };
};