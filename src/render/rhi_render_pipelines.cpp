#include "rhi.h"

#include <vulkan/vulkan.h>

#define MAX_RENDER_PIPELINE_COUNT 100u

struct Render::sBackend {
    bool empty_render_pipelines[MAX_RENDER_PIPELINE_COUNT] = {false};
    sRenderPipeline render_pipelines[MAX_RENDER_PIPELINE_COUNT];
};

VkPipelineColorBlendAttachmentState get_blending_config_additive();
VkPipelineColorBlendAttachmentState get_blending_config_disable();
VkPipelineColorBlendAttachmentState get_blending_config_alphablend();

struct sRenderPipeline {
    VkPipeline vk_pipelines[Render::BLEND_MODE_COUNT * Render::CULL_MODE_COUNT];

    VkPipeline fetch_vulkan_pipeline(   const Render::eBlendMode mode, 
                                        const Render::eCullMode cull) {
        return vk_pipelines[(mode << Render::CULL_MODE_COUNT) | cull];
    }
};

Render::tRenderPipelineId Render::create_render_pipeline(   Render::sBackend* backend, 
                                                            const Render::sCreateRenderPipeline &create_info,
                                                            const sDepthConfig depth = {}, 
                                                            const sMultisampleConfig multisample_config = {}) {
    assert_msg(create_info.color_attachment_count < MAX_COLOR_ATTACHMENT_COUNT, "Too much color attachmetns to pipeline");

    uint8_t sample_count = multisample_config.sample_count;
    assert_msg((sample_count == 1u) || (sample_count > 1u && sample_count % 2u && sample_count <= 64u), "Invalid sample count");

    uint32_t empty_pipeline_index = 0u;
    for(; empty_pipeline_index < MAX_RENDER_PIPELINE_COUNT; empty_pipeline_index++) {
        // TODO: continue here
    }

    VkPipelineInputAssemblyStateCreateInfo      input_assembly_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .topology = create_info.topology,
        .primitiveRestartEnable = false
    };

    VkPipelineViewportStateCreateInfo           viewport_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .viewportCount = 1u,
        .scissorCount = 1u
    };

    VkPipelineRasterizationStateCreateInfo      rasterization_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .polygonMode = create_info.mode,
        .frontFace = create_info.front_face,
        .depthBiasEnable = false,
        .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo        multisample_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .rasterizationSamples = (VkSampleCountFlagBits) sample_count,
        .sampleShadingEnable = multisample_config.enable_min_sample,
        .minSampleShading = multisample_config.min_sample,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    VkPipelineDepthStencilStateCreateInfo       depth_stencil_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .depthTestEnable = depth.enable_depth_test,
        .depthWriteEnable = depth.enable_write_test,
        .depthCompareOp = depth.compare_op,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE, // TODO
        .front = {},
        .back = {},
        .minDepthBounds = 0.0f, // Unused until depth bounds are enabled
        .maxDepthBounds = 1.0f
    };

    // Prepare the dynamic state
    VkDynamicState dynamic_states[2u] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo            dynamic_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .dynamicStateCount = 2u,
        .pDynamicStates = dynamic_states
    };

    VkPipelineColorBlendAttachmentState         blend_state = {};
    VkPipelineColorBlendStateCreateInfo         color_blend_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY, // TODO: review
        .attachmentCount = create_info.color_attachment_count,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
    };


    VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &render_info, // Why here??
        .flags = 0u,
        .stageCount = shader_stages_count,
        .pStages = shader_stages,
        .pVertexInputState = &vertex_input_state,
        .pInputAssemblyState = &input_assembly_state,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterization_state,
        .pMultisampleState = &multisample_state,
        .pDepthStencilState = &depth_stencil_state,
        .pColorBlendState = &color_blend_state,
        .pDynamicState = &dynamic_info,
        .layout = pipeline_layout
    };

    for(uint8_t i = 0u; i < Render::BLEND_MODE_COUNT; i++) {
        switch((Render::eBlendMode) i) {
            case Render::NO_BLEND:
                blend_state = get_blending_config_disable();
            break;
            case Render::ALPHA_BLEND:
                blend_state = get_blending_config_alphablend();
            break;
            case Render::ADDITIVE_BLEND:
                blend_state = get_blending_config_additive();
            break;
            default:
                // ASSERT
        }

        color_blend_state.pAttachments = &blend_state;

        for(uint8_t j = 0u; j < Render::CULL_MODE_COUNT; j++) {
            rasterization_state.cullMode = j;

            pipeline_manager->render_pipelines.vk_pipelines[(i << Render::CULL_MODE_COUNT) | j]

            if (vkCreateGraphicsPipelines(  device, 
                                    VK_NULL_HANDLE, 
                                    1u, 
                                    &pipeline_info, 
                                    nullptr, 
                                    &new_pipeline) != VK_SUCCESS) {
        spdlog::error("Error creating the render pipeline");

        return VK_NULL_HANDLE;
    }
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