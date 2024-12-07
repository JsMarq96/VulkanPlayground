#include "mesh_parser.h"

#include "../render/mesh.h"
#include "../render/renderer.h"

#include <glm/glm.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>

const char* get_directory_of_file(const char* file_dir);

// This only loads the meshes, do not care for the scene part of the GLTF
uint32_t Parsers::gltf_to_mesh(const char* gltf_file_dir, const char* gltf_directory, Render::sGPUMesh meshes_to_fill[100u], Render::sBackend *renderer, Render::sFrame &frame_to_upload) {
    uint32_t mesh_count = 0u;

    fastgltf::GltfDataBuffer data;
    fastgltf::Parser parser(fastgltf::Extensions::None);

    auto gltf_file = fastgltf::MappedGltfFile::FromPath(gltf_file_dir);
    // ! gltf_file

    auto load_result = parser.loadGltf(gltf_file.get(), gltf_directory, fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers | fastgltf::Options::GenerateMeshIndices);

    // asset.error() != fastgltf::Error::None

    // !load
    fastgltf::Asset &gltf = load_result.get();

    uint32_t total_index_buffer_size = 0u;
    uint32_t *tmp_index_buffer = nullptr;

    uint32_t total_vertex_buffer_size = 0u;
    Render::sVertex *tmp_vertex_buffer = nullptr;
    for(fastgltf::Mesh& mesh : gltf.meshes) {
        // Our meshes are the gltf's primitives
        uint32_t mesh_index_count = 0u;
        uint32_t mesh_vertex_count = 0u;
        uint32_t index_idx = 0u;
        uint32_t vertex_idx = 0u;
        for (auto&& p : mesh.primitives) {
            const fastgltf::Accessor& index_accessor = (gltf.accessors[0u]);
            mesh_index_count += (uint32_t) (gltf.accessors[p.indicesAccessor.value()].count);
            mesh_vertex_count += (uint32_t) (gltf.accessors[p.findAttribute("POSITION")->accessorIndex].count);
        }

        // Resize if necesary the index or vertex buffers
        {
            if (mesh_index_count > total_index_buffer_size) {
                total_index_buffer_size = mesh_index_count;

                if (!tmp_index_buffer) {
                    free(tmp_index_buffer);
                }
                tmp_index_buffer = (uint32_t*) malloc(sizeof(uint32_t) * total_index_buffer_size);
            }

            if (mesh_vertex_count > total_vertex_buffer_size) {
                total_vertex_buffer_size = mesh_vertex_count;

                if (!tmp_vertex_buffer) {
                    free(tmp_vertex_buffer);
                }
                tmp_vertex_buffer = (Render::sVertex*) malloc(sizeof(Render::sVertex) * total_vertex_buffer_size);
            }
        }

        for (auto&& p : mesh.primitives) {
            fastgltf::iterateAccessorWithIndex<uint32_t>(gltf, gltf.accessors[p.indicesAccessor.value()],
                    [&tmp_index_buffer](uint32_t idx, size_t buffer_idx) {
                        tmp_index_buffer[buffer_idx] = idx;
                    });

            fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[p.findAttribute("POSITION")->accessorIndex],
                    [&tmp_vertex_buffer](glm::vec3 pos, size_t buffer_idx) {
                        tmp_vertex_buffer[buffer_idx].position = pos;
                    });
            fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[p.findAttribute("NORMAL")->accessorIndex],
                    [&tmp_vertex_buffer](glm::vec3 pos, size_t buffer_idx) {
                        tmp_vertex_buffer[buffer_idx].normal = pos;
                        tmp_vertex_buffer[buffer_idx].color = {pos.x, pos.y, pos.z, 1.0f};
                    });
            fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[p.findAttribute("TANGENT")->accessorIndex],
                    [&tmp_vertex_buffer](glm::vec3 tan, size_t buffer_idx) {
                        tmp_vertex_buffer[buffer_idx].tangent = tan;
                    });
            fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[p.findAttribute("TEXCOORD_0")->accessorIndex],
                    [&tmp_vertex_buffer](glm::vec2 uv, size_t buffer_idx) {
                        tmp_vertex_buffer[buffer_idx].uv = uv;
                    });
            fastgltf::Attribute *vertex_color = p.findAttribute("COLOR_0");
            if (vertex_color) {
                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[vertex_color->accessorIndex],
                    [&tmp_vertex_buffer](glm::vec3 color, size_t buffer_idx) {
                        tmp_vertex_buffer[buffer_idx].color = glm::vec4(color, 1.0f);
                    });
            }
        }

        renderer->create_gpu_mesh(  &meshes_to_fill[mesh_count++], 
                                    tmp_index_buffer, 
                                    mesh_index_count, 
                                    tmp_vertex_buffer, 
                                    mesh_vertex_count, 
                                    frame_to_upload );
    }

    free(tmp_index_buffer);
    free(tmp_vertex_buffer);

    return mesh_count;
}