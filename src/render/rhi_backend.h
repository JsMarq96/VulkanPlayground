#pragma once

#include <cstdint>

#define MAX_RENDER_PIPELINE_COUNT 100u

namespace Render {

    struct sRenderPipeline;
    struct sDescriptorSetAllocator;

    struct sBackend {
        uint64_t device = 0u;
        bool render_pipeline_is_empty[MAX_RENDER_PIPELINE_COUNT] = {true};
        sRenderPipeline *render_pipelines = nullptr;
        sDescriptorSetAllocator *descriptor_set_allocator = nullptr;
    };

    void init_render_pipelines(sBackend *render_backend);
    void init_descriptor_set_allocator(sBackend *render_backend);
};