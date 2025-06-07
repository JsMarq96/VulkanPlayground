#include "rhi.h"

#include <vulkan/vulkan.h>

#define MAX_RENDER_PIPELINE_COUNT 100u

VkPipelineColorBlendAttachmentState get_blending_config_additive();
VkPipelineColorBlendAttachmentState get_blending_config_disable();
VkPipelineColorBlendAttachmentState get_blending_config_alphablend();

struct sRenderPipeline {
    VkPipeline vk_pipelines[BLEND_MODE_COUNT * CULL_MODE_COUNT];

    VkPipeline fetch_vulkan_pipeline(   const Render::sRenderPipeline &pipeline, 
                                        const Render::eBlendMode mode, 
                                        const Render::eCullMode cull) {
        return vk_pipelines[(mode << CULL_MODE_COUNT) | cull];
    }
};

struct sRenderPipelinesManages {
    sRenderPipeline render_pipelines[MAX_RENDER_PIPELINE_COUNT];
};

Render::tRenderPipelineId Render::create_render_pipeline(   Render::sBackend* backend, 
                                                            const Render::sCreateRenderPipeline &create_info) {
    //
    sRenderPipelinesManages *pipeline_manager = backend->render_pipelines;

    VkPipelineInputAssemblyStateCreateInfo      input_assembly_state = {};
    VkPipelineTessellationStateCreateInfo       tessellation_state = {};
    VkPipelineViewportStateCreateInfo           viewport_state = {};
    VkPipelineRasterizationStateCreateInfo      rasterization_state = {};
    VkPipelineMultisampleStateCreateInfo        multisample_state = {};
    VkPipelineDepthStencilStateCreateInfo       depth_stencil_state = {};
    VkPipelineDynamicStateCreateInfo            dynamic_state = {};
    VkPipelineColorBlendAttachmentState         blend_state = {};

    for(uint8_t i = 0u; i <  BLEND_MODE_COUNT; i++) {
        switch((eBlendMode) i) {
            case NO_BLEND:
                blend_state = get_blending_config_disable();
            break;
            case ALPHA_BLEND:
                blend_state = get_blending_config_alphablend();
            break;
            case ADDITIVE_BLEND:
                blend_state = get_blending_config_additive();
            break;
            default:
                // ASSERT
        }

        color_blend_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY, // TODO: review
            .attachmentCount = color_attachment_count,
            .pAttachments = &color_blend_attachment_state,
            .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
        };

        for(uint8_t j = 0u; j <  CULL_MODE_COUNT; j++) {

        }
    }
}


void Render::bind_render_pipeline(  Render::sBackend* backend, 
                                    const Render::tRenderPipelineId pipeline_id, 
                                    const Render::eCullMode cull, 
                                    const Render::eBlendMode blend) {
    //
}




// AUX FUNC ===============

// BLENDING CONFIGS
VkPipelineColorBlendAttachmentState get_blending_config_additive() {
    // result_color = src * src.alpha + dst * 1.0
    return {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
}

VkPipelineColorBlendAttachmentState get_blending_config_alphablend() {
    // result_color = src * src.alpha + dst * (1.0 - src.alpha)
    return {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
    };
}

VkPipelineColorBlendAttachmentState get_blending_config_disable() {
    return {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
}