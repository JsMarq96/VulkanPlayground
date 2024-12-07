#pragma once

#include "../render/gpu_mesh.h"
#include "../render/renderer.h"

namespace Parsers {
    uint32_t gltf_to_mesh(const char* gltf_file_dir, const char* gltf_directory, Render::sGPUMesh meshes_to_fill[100u], Render::sBackend *renderer, Render::sFrame &frame_to_upload);
};