#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace Render {
    struct sGPUBuffer {
        VkBuffer buffer;
        VmaAllocation alloc;
        VmaAllocationInfo alloc_info;
    };

    struct sGPUBufferView {
        sGPUBuffer *raw_buffer = nullptr;
        size_t offset = 0u;
        size_t size = 0u;
    };
};