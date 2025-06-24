#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>

#include "rhi_types.h"

#include "resources/image_formats.h"

namespace Render {


    struct sBackend;

    sBackend* create_render_backend(const uint64_t gpu_device);

    // DESCRIPTOR SETS =======================
    #define MAX_BIDING_COUNT 16u
    #define DEFAULT_DESCRIPTOR_SET_PER_POOL_COUNT 100u

    // Ratios from https://github.com/keengames/vulkan_backend/blob/3b0c25c2425a9e2170461211c272b8f56337d2e2/code/vulkan_graphics_objects.cpp#L49
    struct sDescriptorPoolSizes {
        uint32_t descriptor_set_count = DEFAULT_DESCRIPTOR_SET_PER_POOL_COUNT;
        uint32_t uniform_buffer_count = DEFAULT_DESCRIPTOR_SET_PER_POOL_COUNT * 2u;
        uint32_t storage_buffer_count = DEFAULT_DESCRIPTOR_SET_PER_POOL_COUNT * 2u;
        uint32_t sampler_count = DEFAULT_DESCRIPTOR_SET_PER_POOL_COUNT * 4u;
        uint32_t sampled_image_count = DEFAULT_DESCRIPTOR_SET_PER_POOL_COUNT * 16u;
        uint32_t storage_image_count = DEFAULT_DESCRIPTOR_SET_PER_POOL_COUNT * 4u;
    };

    struct sDescriptorSetBiding {
        uint8_t index = 0u;
        VkDescriptorType type;
    };

    struct sDescriptorSetLayoutParams {
        VkShaderStageFlags shader_stage = 0u;
        void* p_next = nullptr;
        uint32_t create_flags = 0u;
        uint32_t biding_count = 0u;
        sDescriptorSetBiding bidings[MAX_BIDING_COUNT];
    };

    VkDescriptorSetLayout create_descriptor_set_layout(sBackend *backend, const sDescriptorSetLayoutParams &create_ds_layout);

    tGPUDescriptorPoolId create_descriptor_pool(sBackend *backend);

    tGPUDescriptorPoolId create_descriptor_pool_with_custom_sizes(sBackend *backend, const sDescriptorPoolSizes pool_sizes);

    void delete_descriptor_pool(sBackend *backend, const tGPUDescriptorPoolId pool_id);

    void clear_descriptor_pool(sBackend *backend, const tGPUDescriptorPoolId pool_id);

    bool alloc_descriptor_set_in_pool(sBackend *backend, const tGPUDescriptorPoolId pool_id, const VkDescriptorSetLayout layout, VkDescriptorSet *result);

    // RENDER PIPELINES =======================
    enum eBlendMode : uint32_t {
        NO_BLEND = 0u,
        ALPHA_BLEND,
        ADDITIVE_BLEND,
        BLEND_MODE_COUNT
    };

    enum eCullMode : uint32_t {
        NONE = 0u,
        FRONT,
        BACK,
        FRONT_AND_BACK,
        CULL_MODE_COUNT
    };

    #define MAX_COLOR_ATTACHMENT_COUNT 4u
    #define SHADER_STAGE_COUNT 4u

    
    struct sRenderPipelineParams {
        VkShaderModule vertex_shader;
        VkShaderModule fragment_shader;
        VkPolygonMode mode = VK_POLYGON_MODE_FILL;
        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkFormat depth_format;
        VkFormat stencil_format;
        VkFrontFace front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        uint32_t color_attachment_count = 0u;
        VkFormat color_attachments_format[MAX_COLOR_ATTACHMENT_COUNT] = {};
        uint32_t    view_mask = 0u;
        VkPipelineLayout pipeline_layout;
    };

    struct sRenderPipelineDepthParams {
        bool enable_depth_test = true;
        bool enable_write_test = true;
        VkCompareOp compare_op = VK_COMPARE_OP_LESS_OR_EQUAL;
    };

    struct sRenderPipelineMultisamplingParams {
        uint8_t sample_count = 1u;
        bool enable_min_sample = false;
        float min_sample = 1.0f;
    };

    tRenderPipelineId create_render_pipeline(sBackend* backend, const sRenderPipelineParams &create_info, const sRenderPipelineDepthParams depth, const sRenderPipelineMultisamplingParams multisample_config);

    void bind_render_pipeline(sBackend* backend, const tRenderPipelineId pipeline_id, const eCullMode cull, const eBlendMode blend);

    // SWAPCHAIN ========================================

    struct sSwapchainParams {
        uint32_t width = 0u;
        uint32_t height = 0u;
        VkFormat format;
        VkColorSpaceKHR color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        VkSurfaceKHR surface;
        VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
        VkImageUsageFlagBits img_usage_bits = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    };

    struct sSwapchain;

    bool create_swapchain(sBackend* backend, sSwapchainParams &swapchain_params);
    void delete_swapchain(sBackend* backend);

    VkImage swapchain_adquire_next_img(sBackend* backend);

    tGPUBufferId create_gpu_buffer();
};