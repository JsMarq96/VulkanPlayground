#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "mesh.h"
#include "gpu_buffers.h"

namespace Render {
    struct sVertex {
        glm::vec3   position = {0.0f, 0.0f, 0.0f};
        float       pad1;
        glm::vec3   normal = {0.0f, 0.0f, 0.0f};
        float       pad2;
        glm::vec2   uv = {0.0f, 0.0f};
        glm::vec2   pad3 = {0.0f, 0.0f};
        glm::vec3   tangent = {0.0f, 0.0f, 0.0f};
        float       pad0;
        glm::vec4   color = {0.0f, 0.0f, 0.0f, 1.0f};
    };

    struct sVertexSimple {
        glm::vec3   position;
        float       uv_x;
        glm::vec3   normal;
        float       uv_y;
        glm::vec4   color;
    };
    
    struct sGPUMesh {
        sGPUBuffer      index_buffer;
        sGPUBuffer      vertex_buffer;

        uint32_t        index_count;

        VkDeviceAddress vertex_buffer_address;
    };

    struct sMeshPushConstant {
        glm::mat4           mvp_matrix;
        VkDeviceAddress     vertex_buffer;
    };
};