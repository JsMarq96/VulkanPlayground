#pragma once

#include <stdint.h>
#include <vulkan/vulkan.h>

#define MAX_BIDING_COUNT 8u

// Namespace for all vulkan helper functions
namespace VK_Helpers {
    // Structures

    struct sDescriptorLayoutBuilder {
        // Assemble Descriptor set layouts with the builder pattern
        VkDescriptorSetLayoutBinding descriptor_pairs[MAX_BIDING_COUNT];
        uint8_t descriptor_count = 0u;

        VkDevice descriptor_device;
        VkShaderStageFlags descriptor_shader_stage;
        VkDescriptorSetLayoutCreateFlags create_flags = 0u;
        void* p_next = nullptr;

        static sDescriptorLayoutBuilder create( const VkDevice device, 
                                                const VkShaderStageFlags shader_stage, 
                                                void* p_next = nullptr, 
                                                const VkDescriptorSetLayoutCreateFlags flags = 0u);
        sDescriptorLayoutBuilder& add_biding(const uint8_t binding, const VkDescriptorType type);
        VkDescriptorSetLayout build();
    };

    // Cmd pool
    VkCommandPoolCreateInfo create_cmd_pool_info(const uint32_t queue_family_index, const VkCommandPoolCreateFlags flags = 0u);

    // Cmd buffer
    VkCommandBufferAllocateInfo create_cmd_buffer_alloc_info(const VkCommandPool pool, const uint32_t count);
    VkCommandBufferBeginInfo create_cmd_buffer_begin_info(const VkCommandBufferUsageFlags flags = 0u);
    VkCommandBufferSubmitInfo  create_cmd_buffer_submit_info(const VkCommandBuffer &cmd);
    VkSubmitInfo2 create_cmd_submit(const VkCommandBufferSubmitInfo *cmd, const VkSemaphoreSubmitInfo *signal_semaphore_info, const VkSemaphoreSubmitInfo *wait_semphore_info);

    // Fences
    VkFenceCreateInfo create_fence_info(const VkFenceCreateFlags flags = 0u);

    // Semaphores
    VkSemaphoreCreateInfo create_semaphore_info(const VkSemaphoreCreateFlags flags = 0u);
    VkSemaphoreSubmitInfo create_submit_semphore_info(const VkPipelineStageFlags2 stage_mask, const VkSemaphore semaphore);

    // Images
    void transition_image_layout(const VkCommandBuffer cmd, const VkImage image, const VkImageLayout current_layout, const VkImageLayout new_layout);
    VkImageSubresourceRange image_subresource_range(const VkImageAspectFlags aspect_flags = 0u);
    VkImageCreateInfo image2D_create_info(const VkFormat format, const VkImageUsageFlags usage_flags, const VkExtent3D extent, const bool use_on_CPU = false);
    VkImageViewCreateInfo image_view2D_create_info(const VkFormat format, const VkImage &image, const VkImageAspectFlags &aspect_flags);
    // TODO: copy also with vkCmdCopyImage (more perfomant)
    void copy_image_image(const VkCommandBuffer cmd, const VkImage src, const VkExtent3D src_size, const VkImage dst, const VkExtent3D dst_size);

    // Descritors

}