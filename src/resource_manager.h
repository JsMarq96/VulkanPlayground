#pragma once

#include <cstdint>

#include "utils/arena_list.h"
#include "render/gpu_buffers.h"
#include "render/gpu_mesh.h"
#include "render/resources.h"

typedef uint64_t ResourceId_t;

enum eResourceType : uint8_t {
    RESOURCE_TYPE_MESH = 0u,
    RESOURCE_TYPE_GPU_BUFFER,
    RESOURCE_TYPE_IMAGE,
    RESOURCE_TYPE_COUNT
};

struct sResourceManager {
    static sResourceManager* instance;

    sArenaList<Render::sGPUBuffer, 100u> gpu_buffer_arena;
    sArenaList<Render::sGPUMesh, 100u> gpu_mesh_arena;
    sArenaList<sImage, 100u> gpu_image_arena;

    static inline eResourceType get_resource_type(const ResourceId_t id) {
        return (eResourceType)((uint8_t) id >> 57u);
    }

    static inline uint64_t get_resource_area_idx(const ResourceId_t id) {
        return 0x00FFFFFFFFFFFFFFu & id;
    }

    template<typename T>
    static T& get(const ResourceId_t id) {
        if (typeid(T) == typeid(Render::sGPUBuffer)) {
            return sResourceManager::instance->gpu_buffer_arena.get(get_resource_area_idx(id));
        } else if (typeid(T) == typeid(Render::sGPUMesh)) {
            return sResourceManager::instance->gpu_mesh_arena.get(get_resource_area_idx(id));
        } else if (typeid(T) == typeid(sImage)) {
            return sResourceManager::instance->gpu_image_arena.get(get_resource_area_idx(id));
        }
    }
};