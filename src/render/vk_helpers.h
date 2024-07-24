#pragma once

#include <stdint.h>
#include <vulkan/vulkan.h>

// Namespace for all vulkan helper functions
namespace VK_Helpers {
    // Cmd pool
    VkCommandPoolCreateInfo create_cmd_pool_info(const uint32_t queue_family_index, const VkCommandPoolCreateFlags flags = 0u);

    // Cmd buffer
    VkCommandBufferAllocateInfo create_cmd_buffer_alloc_info(const VkCommandPool pool, const uint32_t count);
    VkCommandBufferBeginInfo create_cmd_buffer_begin_info(const VkCommandBufferUsageFlags flags = 0u);

    // Fences
    VkFenceCreateInfo create_fence_info(const VkFenceCreateFlags flags = 0u);

    // Semaphores
    VkSemaphoreCreateInfo create_semaphore_info(const VkSemaphoreCreateFlags flags = 0u);
    VkSemaphoreSubmitInfo submit_semphore_info(const VkPipelineStageFlags2 stage_mask, const VkSemaphore semaphore);

    // Images
    void transition_image_layout(const VkCommandBuffer cmd, const VkImage image, const VkImageLayout current_layout, const VkImageLayout new_layout);
    VkImageSubresourceRange image_subresource_range(const VkImageAspectFlags aspect_flags = 0u);
}