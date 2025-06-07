#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>
#include <cstdarg>

#include "rhi_types.h"

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
    
    struct sCreateRenderPipeline {
        VkShaderModule vertex_shader;
        VkShaderModule fragment_shader;
        VkPolygonMode mode;
        VkPrimitiveTopology topology;
        eImageFormats depth_format;
        eImageFormats stencil_format;
        VkFrontFace front_face;
        // TODO color attachments
        // 
    };

    tRenderPipelineId create_render_pipeline(sBackend* backend, const sCreateRenderPipeline &create_info);

    void bind_render_pipeline(sBackend* backend, const tRenderPipelineId pipeline_id, const eCullMode cull, const eBlendMode blend);

    tGPUBufferId create_gpu_buffer();
};