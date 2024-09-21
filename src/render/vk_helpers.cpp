#include "vk_helpers.h"

VkCommandPoolCreateInfo VK_Helpers::create_cmd_pool_info(   const uint32_t queue_family_index, 
                                                            const VkCommandPoolCreateFlags flags ) {
    return {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags,
        .queueFamilyIndex = queue_family_index
    };
}

VkCommandBufferAllocateInfo VK_Helpers::create_cmd_buffer_alloc_info(   const VkCommandPool pool, 
                                                                        const uint32_t count ) {
    return {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = count
    };
}

VkCommandBufferBeginInfo VK_Helpers::create_cmd_buffer_begin_info(const VkCommandBufferUsageFlags flags ) {
    return {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = flags,
        .pInheritanceInfo = nullptr
    };
}

VkCommandBufferSubmitInfo  VK_Helpers::create_cmd_buffer_submit_info(const VkCommandBuffer &cmd) {
    return {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .pNext = nullptr,
        .commandBuffer = cmd,
        .deviceMask = 0u
    };
}

VkFenceCreateInfo VK_Helpers::create_fence_info( const VkFenceCreateFlags flags ) {
    return {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags
    };
}

VkSemaphoreCreateInfo VK_Helpers::create_semaphore_info( const VkSemaphoreCreateFlags flags ) {
    return {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = flags
    };
}

VkSemaphoreSubmitInfo VK_Helpers::create_submit_semphore_info(const VkPipelineStageFlags2 stage_mask, const VkSemaphore semaphore) {
    return {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .pNext = nullptr,
        .semaphore = semaphore,
        .value = 1u, // To send the semaphore
        .stageMask = stage_mask,
        .deviceIndex = 0u
    };
}

VkSubmitInfo2 VK_Helpers::create_cmd_submit(    const VkCommandBufferSubmitInfo *cmd, 
                                                const VkSemaphoreSubmitInfo *signal_semaphore_info, 
                                                const VkSemaphoreSubmitInfo *wait_semphore_info) {
    return {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .pNext = nullptr,
        
        .waitSemaphoreInfoCount = (wait_semphore_info == nullptr) ? 0u : 1u,
        .pWaitSemaphoreInfos = wait_semphore_info,

        .commandBufferInfoCount = 1u,
        .pCommandBufferInfos = cmd,

        .signalSemaphoreInfoCount = (signal_semaphore_info == nullptr) ? 0u : 1u,
        .pSignalSemaphoreInfos = signal_semaphore_info
    };
}

// TODO: not optimun, due the barriers it locks the whole GPU
// Best use the StageMasks more finelly 
// https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples
void VK_Helpers::transition_image_layout(   const VkCommandBuffer cmd, 
                                            const VkImage image, 
                                            const VkImageLayout old_layout, 
                                            const VkImageLayout new_layout) {
    VkImageAspectFlags aspect_mask = (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    
    // Using vulkan 1.3 Pipeline barrier Synchronization
    VkImageMemoryBarrier2 image_barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .pNext = nullptr,

        .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,

        .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,

        .oldLayout = old_layout,
        .newLayout = new_layout,
        
        .image = image,
        .subresourceRange = VK_Helpers::image_subresource_range(aspect_mask),
    };

    VkDependencyInfo dep_info = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext = nullptr,

        .imageMemoryBarrierCount = 1u,
        .pImageMemoryBarriers = &image_barrier
    };

    vkCmdPipelineBarrier2(cmd, &dep_info);
}

// This is for locking areas and mip levels of an image
// Use  e.g. case: mipmap generation
VkImageSubresourceRange VK_Helpers::image_subresource_range( const VkImageAspectFlags aspect_flags ) {
    return {
        .aspectMask = aspect_flags,
        .baseMipLevel = 0u,
        .levelCount = VK_REMAINING_MIP_LEVELS,
        .baseArrayLayer = 0u,
        .layerCount = VK_REMAINING_ARRAY_LAYERS
    };
}

VkImageCreateInfo VK_Helpers::image2D_create_info(  const VkFormat format, 
                                                    const VkImageUsageFlags usage_flags, 
                                                    const VkExtent3D extent, 
                                                    const bool use_on_CPU) {
    return {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = extent,
        .mipLevels = 1u,
        .arrayLayers = 1u,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = (use_on_CPU) ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL, // best fomat, if we read from CPU is liner
        .usage = usage_flags
    };
}

VkImageViewCreateInfo VK_Helpers::image_view2D_create_info( const VkFormat format, 
                                                            const VkImage &image, 
                                                            const VkImageAspectFlags &aspect_flags) {
    return {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange = {
            .aspectMask = aspect_flags,
            .baseMipLevel = 0u,
            .levelCount = 1u,
            .baseArrayLayer = 0u,
            .layerCount = 1u
        }
    };
}