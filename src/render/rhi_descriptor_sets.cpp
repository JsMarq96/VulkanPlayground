#include "rhi.h"

#include "vk_helpers.h"

#include <cstdarg>

#define MAX_DS_POOL_COUNT 20u

// LAYOUT DESCRIPTOR BUILDER ===============
VkDescriptorSetLayout Render::create_descriptor_set_layout( Render::sBackend *backend, 
                                                            const sCreateDescriptorSetLayout &create_ds_layout) {
    VkDescriptorSetLayoutBinding descriptor_pairs[MAX_BIDING_COUNT];

    VkDescriptorSetLayoutCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = create_ds_layout.p_next,
        .flags = create_ds_layout.create_flags,
        .bindingCount = create_ds_layout.biding_count,
        .pBindings = descriptor_pairs
    };

    VkDescriptorSetLayout resulting_set_layout = {};

    for(uint32_t i = 0u; i < create_ds_layout.biding_count; i++) {
        descriptor_pairs[i] = {
            .binding = create_ds_layout.bidings[i].index,
            .descriptorType = create_ds_layout.bidings[i].type,
            .descriptorCount = 1u,
            .stageFlags = create_ds_layout.shader_stage
        };
    }

    vk_assert_msg(  vkCreateDescriptorSetLayout(descriptor_device, 
                                                &info, 
                                                nullptr, 
                                                &resulting_set_layout),
                    "Error creating descritpor set layout");

    return resulting_set_layout;
}

// DESCRIPTOR SET ALLOCATOR =======

struct Render::sDescriptorSetAllocator {
    bool empty_descriptor_pools[MAX_DS_POOL_COUNT] = {true};
    VkDescriptorPool descriptor_pools[MAX_DS_POOL_COUNT] = {};
};

void Render::init_descriptor_set_allocator(Render::sBackend *render_backend) {
    render_backend->descriptor_set_allocator = (sDescriptorSetAllocator*) malloc(sizeof(sDescriptorSetAllocator));
    memset(render_backend->descriptor_set_allocator, true, sizeof(bool) * MAX_DS_POOL_COUNT);
}

Render::tGPUDescriptorPoolId Render::create_descriptor_pool(sBackend *backend) {
    sDescriptorPoolSizes default_sizes = {};
    return create_descriptor_pool_with_custom_sizes(backend, default_sizes);
}

Render::tGPUDescriptorPoolId Render::create_descriptor_pool_with_custom_sizes(  Render::sBackend *backend, 
                                                                                const sDescriptorPoolSizes pool_sizes) {
    uint8_t new_pool_idx = 0u;
    for(;new_pool_idx < MAX_DS_POOL_COUNT; i++) {
        if (empty_descriptor_pools[new_pool_idx]) {
            break;
        }
    }

    assert_msg(new_pool_idx == MAX_DS_POOL_COUNT, "Allocated too much pools");

    VkDescriptorPoolSize vk_pool_sizes[5u] = {
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = pool_sizes.sampler_count
        },
        {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = pool_sizes.storage_image_count
        },
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = pool_sizes.uniform_buffer_count
        },
        {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = pool_sizes.storage_buffer_count,
        },
        {
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = pool_sizes.sampled_image_count
        }
    };

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .maxSets = pool_sizes.descriptor_set_count,
        .poolSizeCount = 5u,
        .pPoolSizes = vk_pool_sizes
    };

    vk_assert_msg(  vkCreateDescriptorPool( device, 
                                            &pool_info, 
                                            nullptr, 
                                            &descriptor_pools[new_pool_idx]),
                    "Error creating initial descriptor pool");
    
    return new_pool_idx;
}
    
void Render::delete_descriptor_pool(Render::sBackend *backend, 
                                    const Render::tGPUDescriptorPoolId pool_id) {
    if (empty_descriptor_pools[pool_id]) {
        return;
    }

    vkDestroyDescriptorPool((VkDevice) backend->device, 
                            backend->descriptor_set_allocator, 
                            nullptr);
}

void Render::clear_descriptor_pool( Render::sBackend *backend, 
                                    const Render::tGPUDescriptorPoolId pool_id) {
    if (empty_descriptor_pools[pool_id]) {
        return;
    }

    vkResetDescriptorPool(  (VkDevice) backend->device,
                            backend->descriptor_set_allocator, 
                            0);
}

bool Render::alloc_descriptor_set_in_pool(  sRender::Backend *backend, 
                                            const Render::tGPUDescriptorPoolId pool_id, 
                                            const VkDescriptorSetLayout layout,
                                            VkDescriptorSet *result) {
    VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = backend->descriptor_set_allocator->descriptor_pools[pool_id],
        .descriptorSetCount = 1u,
        .pSetLayouts = &layout
    };

    VkResult result = vkAllocateDescriptorSets(pool_device, &alloc_info, result);

    if (result != VK_ERROR_OUT_OF_POOL_MEMORY && result != VK_ERROR_FRAGMENTED_POO && result != VK_SUCCESS) {
        assert_msg(false, "Error allocating descriptors");
    }

    return result != VK_SUCCESS;
}