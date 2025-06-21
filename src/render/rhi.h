#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>
#include <cstdarg>

#include "rhi_types.h"

#include "resources/image_formats.h"

namespace Render {


    struct sBackend;

    // Descriptor sets =======================

    struct sDescriptorSetBiding {
        uint8_t index = 0u;
        VkDescriptorType type;
    };

    VkDescriptorSetLayout create_descriptor_set_layout(sBackend *backend, const sDescriptorSetBiding bidings...);

    struct sDescriptorLayout {
        VkDescriptorSetLayout layout;
    };
    void allocate_descriptor_set(sBackend *backend, const VkDescriptorSetLayout &layout);


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

    
    struct sCreateRenderPipeline {
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

    struct sRenderPipelineDepthConfig {
        bool enable_depth_test = true;
        bool enable_write_test = true;
        VkCompareOp compare_op = VK_COMPARE_OP_LESS_OR_EQUAL;
    };

    struct sRenderPipelineMultisamplingConfig {
        uint8_t sample_count = 1u;
        bool enable_min_sample = false;
        float min_sample = 1.0f;
    };

    tRenderPipelineId create_render_pipeline(sBackend* backend, const sCreateRenderPipeline &create_info, const sRenderPipelineDepthConfig depth, const sRenderPipelineMultisamplingConfig multisample_config);

    void bind_render_pipeline(sBackend* backend, const tRenderPipelineId pipeline_id, const eCullMode cull, const eBlendMode blend);

    tGPUBufferId create_gpu_buffer();
};