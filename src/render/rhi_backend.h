#pragma once

#include <cstdint>

#define MAX_RENDER_PIPELINE_COUNT 100u
#define FRAME_BUFFER_COUNT 4u

namespace Render {

    struct sRenderPipeline;
    struct sDescriptorSetAllocator;

    struct sBackend {
        uint64_t device = 0u;
        uint64_t physical_device = 0u;

        struct sSwapchainData {
            uint32_t vk_format;
            uint32_t width = 0u;
            uint32_t height = 0u;
            uint64_t vk_swapchain;
            uint64_t vk_images[FRAME_BUFFER_COUNT];
            uint64_t vk_image_views[FRAME_BUFFER_COUNT];
        } swapchain_data;

        bool render_pipeline_is_empty[MAX_RENDER_PIPELINE_COUNT] = {true};
        sRenderPipeline *render_pipelines = nullptr;

        sDescriptorSetAllocator *descriptor_set_allocator = nullptr;
    };

    void init_render_pipelines(sBackend *render_backend);
    void init_descriptor_set_allocator(sBackend *render_backend);
};