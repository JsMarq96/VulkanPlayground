#include "rhi.h"
#include "rhi_backend.h"

#include <stdlib.h>
#include <string.h>

Render::sBackend* Render::create_render_backend(const uint64_t gpu_device) {
    sBackend *render_backend = (sBackend*) malloc(sizeof(sBackend));

    render_backend->device = gpu_device;
    memset(render_backend->render_pipeline_is_empty, true, sizeof(bool) * MAX_RENDER_PIPELINE_COUNT);

    init_render_pipelines(render_backend);
    init_descriptor_set_allocator(render_backend);

    return render_backend;
}