#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>

#include "gpu_buffers.h"

namespace Render {
    struct sVertex {
        glm::vec3 position;
        float uv_x;
        glm::vec3 normal;
        float uv_y;
        glm::vec4 color;
    };
    
    struct sGPUMesh {
        sGPUBuffer index_buffer;
        sGPUBuffer vertex_buffer;
        VkDeviceAdress vertex_buffer_address;
    };
};