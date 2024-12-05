#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "gpu_buffers.h"

namespace Render {
    struct sVertex {
        glm::vec3   position;
        float       uv_x;
        glm::vec3   normal;
        float       uv_y;
        glm::vec4   color;
    };
    
    struct sGPUMesh {
        sGPUBuffer      index_buffer;
        sGPUBuffer      vertex_buffer;
        VkDeviceAddress vertex_buffer_address;
    };

    struct sMeshPushConstant {
        glm::mat4           mvp_matrix;
        VkDeviceAddress     vertex_buffer;
    };
};