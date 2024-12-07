#pragma once

#include <glm/glm.hpp>

#define MESH_NAME_SIZE 23u
namespace Geometry {
    struct sVertex {
        glm::vec3   position;
        float       pad;
        glm::vec2   normal;
        glm::vec2   uv;
        glm::vec3   tangent;
        float       pad0;
        glm::vec4   color;
    };
    
    struct sSurface {
        uint32_t start_index = 0u;
        uint32_t count = 0u;
    };

    struct sCPUMesh {
        uint32_t index_count = 0u;
        uint32_t* indices = nullptr;

        uint32_t vertex_count = 0u;
        sVertex* vertices = nullptr; 

        char name[MESH_NAME_SIZE];
    };
};
