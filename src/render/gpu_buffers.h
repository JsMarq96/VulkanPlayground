#pragma once

#include <cstddef>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace Render {
    struct sGPUBuffer {
        VkBuffer buffer;
        VmaAllocation alloc;
        VmaAllocationInfo alloc_info;

        size_t size = 0u;
    };

    struct sGPUBufferView {
        sGPUBuffer *raw_buffer = nullptr;
        size_t offset = 0u;
        size_t size = 0u;
    };
};