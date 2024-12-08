#include "pipeline.h"

#include "../utils.h"
#include "vk_helpers.h"

using namespace Render;

void sGraphicsPipelineBuilder::clear() {
    shader_stages_count = 0u;
    enabled_flags = 0u;
}

VkPipeline sGraphicsPipelineBuilder::build(const VkDevice device, const VkPipelineLayout pipeline_layout) {
    assert_msg( (enabled_flags & FULL_MANDATORY_CONFIG) == FULL_MANDATORY_CONFIG,
                "Missing fundamental configurations of the render pipeline");

    rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state.pNext = nullptr;
    rasterization_state.lineWidth = 1.0f;

    VkPipelineRenderingCreateInfo render_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = nullptr,
        .viewMask = view_mask,
        .colorAttachmentCount = color_attachment_count,
        .pColorAttachmentFormats = color_attachments_format,
        .depthAttachmentFormat = depth_attachment_format,
        .stencilAttachmentFormat = stencil_attachment_format
    };

    VkPipelineViewportStateCreateInfo viewport_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .viewportCount = 1u,
        .scissorCount = 1u
    };

    VkPipelineColorBlendStateCreateInfo color_blend_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY, // TODO: review
        .attachmentCount = color_attachment_count,
        .pAttachments = &color_blend_attachment_state,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
    };

    VkPipelineVertexInputStateCreateInfo vertex_input_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .vertexBindingDescriptionCount = 0u,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0u,
        .pVertexAttributeDescriptions = nullptr
    };

    // Prepare the dynamic state
    VkDynamicState dynamic_states[2u] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .dynamicStateCount = 2u,
        .pDynamicStates = dynamic_states
    };

    // TODO: Add renderpass
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

    VkPipeline new_pipeline;

    if (vkCreateGraphicsPipelines(  device, 
                                    VK_NULL_HANDLE, 
                                    1u, 
                                    &pipeline_info, 
                                    nullptr, 
                                    &new_pipeline) != VK_SUCCESS) {
        spdlog::error("Error creating the render pipeline");

        return VK_NULL_HANDLE;
    }

    return new_pipeline;
}

void sGraphicsPipelineBuilder::set_shaders( const VkShaderModule vertex_shader, 
                                            const VkShaderModule fragment_shader) {
    shader_stages_count = 2u;
    shader_stages[0u] = VK_Helpers::shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vertex_shader);
    shader_stages[1u] = VK_Helpers::shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader);

    enabled_flags |= CONFIGURED_SHADERS;
}

void sGraphicsPipelineBuilder::set_topology(const VkPrimitiveTopology topology) {
    input_assembly_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .topology = topology
    };

    enabled_flags |= CONFIGURED_TOPOLOGY;
}

void sGraphicsPipelineBuilder::set_polygon_mode(const VkPolygonMode mode) {
    rasterization_state.polygonMode = mode;
    
    enabled_flags |= CONFIGURED_POLYGON_MODE;
}

void sGraphicsPipelineBuilder::set_cull_mode(const VkCullModeFlags cull_mode, const VkFrontFace front_face) {
    rasterization_state.cullMode = cull_mode;
    rasterization_state.frontFace = front_face;

    enabled_flags |= CONFIGURED_CULL_MODE;
}

void sGraphicsPipelineBuilder::add_color_attachment_format(const VkFormat format) {
    assert_msg(color_attachment_count < PIPELINE_COLOR_ATTACHMENT_MAX_COUNT, "Too much color attachmetns to pipeline");
    color_attachments_format[color_attachment_count++] = format;
}

void sGraphicsPipelineBuilder::set_depth_format(const VkFormat format) {
    depth_attachment_format = format;
    enabled_flags |= CONFIGURED_DEPTH_FORMAT;
}

void sGraphicsPipelineBuilder::set_stencil_format(const VkFormat format) {
    stencil_attachment_format = format;
    enabled_flags |= CONFIGURED_STENCIL_FORMAT;
}

void sGraphicsPipelineBuilder::enable_depth_test(const bool depth_write_enable, const VkCompareOp op) {
    depth_stencil_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = depth_write_enable,
        .depthCompareOp = op,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE, // TODO
        .front = {},
        .back = {},
        .minDepthBounds = 0.0f, // Unused until depth bounds are enabled
        .maxDepthBounds = 1.0f
    };

    enabled_flags |= CONFIGURED_DEPTH_TEST;
}

void sGraphicsPipelineBuilder::disable_depth_test() {
    depth_stencil_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .depthTestEnable = VK_FALSE,
        .depthWriteEnable = VK_FALSE,
        .depthCompareOp = VK_COMPARE_OP_NEVER,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .front = {},
        .back = {},
        .minDepthBounds = 0.0f, // Unused until depth bounds are enabled
        .maxDepthBounds = 1.0f        
    };

    enabled_flags |= CONFIGURED_DEPTH_TEST;
}
void sGraphicsPipelineBuilder::disable_multisampling() {
    multisample_state = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    enabled_flags |= CONFIGURED_MULTISAMPLING;
}

// Disable blending for all channels
void sGraphicsPipelineBuilder::disable_blending() {
    color_blend_attachment_state = {
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    enabled_flags |= CONFIGURED_BLENDING;
}