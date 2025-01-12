#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "resources.h"

#define SHADER_STAGE_COUNT 4u
#define PIPELINE_COLOR_ATTACHMENT_MAX_COUNT 4u

namespace Render {

    struct sGraphicsPipelineBuilder {
        enum ePipelineConfigs : uint32_t {
            CONFIGURED_SHADERS          = 0b1u,
            CONFIGURED_TOPOLOGY         = 0b10u,
            CONFIGURED_CULL_MODE        = 0b100u,
            CONFIGURED_POLYGON_MODE     = 0b1000u,
            CONFIGURED_DEPTH_FORMAT     = 0b10000u,
            CONFIGURED_STENCIL_FORMAT   = 0b100000u,
            CONFIGURED_DEPTH_TEST       = 0b1000000u,
            CONFIGURED_MULTISAMPLING    = 0b10000000u,
            CONFIGURED_BLENDING         = 0b100000000u,
            FULL_MANDATORY_CONFIG       = CONFIGURED_SHADERS |
                CONFIGURED_TOPOLOGY | CONFIGURED_CULL_MODE | 
                CONFIGURED_POLYGON_MODE | CONFIGURED_DEPTH_FORMAT |
                CONFIGURED_STENCIL_FORMAT | CONFIGURED_DEPTH_TEST |
                CONFIGURED_MULTISAMPLING | CONFIGURED_BLENDING
        };

        // TODO: export to multiple attachments
        // TODO: support set blending
        // TODO: support 
        uint16_t                                    enabled_flags = 0u;

        uint32_t                                    shader_stages_count = 0u;
        VkPipelineShaderStageCreateInfo             shader_stages[SHADER_STAGE_COUNT];

        VkPipelineInputAssemblyStateCreateInfo      input_assembly_state = {};
        VkPipelineTessellationStateCreateInfo       tessellation_state = {};
        VkPipelineViewportStateCreateInfo           viewport_state = {};
        VkPipelineRasterizationStateCreateInfo      rasterization_state = {};
        VkPipelineMultisampleStateCreateInfo        multisample_state = {};
        VkPipelineDepthStencilStateCreateInfo       depth_stencil_state = {};
        VkPipelineDynamicStateCreateInfo            dynamic_state = {};
        VkPipelineColorBlendAttachmentState         color_blend_attachment_state = {};

        uint32_t                                    view_mask = 0u;

        uint32_t                                    color_attachment_count = 0u;
        VkFormat                                    color_attachments_format[PIPELINE_COLOR_ATTACHMENT_MAX_COUNT] = {};

        VkFormat                                    depth_attachment_format;
        VkFormat                                    stencil_attachment_format;

        void clear();

        VkPipeline build(const VkDevice device, const VkPipelineLayout pipeline_layout);

        void set_shaders(const VkShaderModule vertex_shader, const VkShaderModule fragment_shader);
        void set_topology(const VkPrimitiveTopology topology);
        void set_polygon_mode(const VkPolygonMode mode);
        void set_cull_mode(const VkCullModeFlags cull_mode, const VkFrontFace front_face);
        void set_depth_format(const eImageFormats format);
        void set_stencil_format(const eImageFormats format);
        void set_depth_test(const bool depth_write_enable, const VkCompareOp op);
        void set_blending_additive();
        void set_blending_alphablend();

        void add_color_attachment_format(const eImageFormats format);

        void disable_depth_test();
        void disable_multisampling();
        void disable_blending();
    };
};