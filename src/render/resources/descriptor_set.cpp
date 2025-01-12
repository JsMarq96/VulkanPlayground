#include "descriptor_set.h"

#include <cstdlib>

#include "../render_utils.h"

// DESCRIPTOR LAYOUT BUILDER =====================
sDescriptorLayoutBuilder sDescriptorLayoutBuilder::create(  const VkDevice device, 
                                                            const VkShaderStageFlags shader_stage, 
                                                            void* p_next, 
                                                            const VkDescriptorSetLayoutCreateFlags flags ) {
    sDescriptorLayoutBuilder builder;
    
    builder.descriptor_device = device;
    builder.descriptor_shader_stage = shader_stage;
    builder.p_next = p_next;
    builder.create_flags = flags;

    return builder;
}

sDescriptorLayoutBuilder& sDescriptorLayoutBuilder::add_biding( const uint8_t binding, 
                                                                const VkDescriptorType type ) {
    descriptor_pairs[descriptor_count++] = {
            .binding = binding,
            .descriptorType = type,
            .descriptorCount = 1u,
            .stageFlags = descriptor_shader_stage
        };

    return *this;
}

VkDescriptorSetLayout sDescriptorLayoutBuilder::build(  void *next_pointer ) {
    VkDescriptorSetLayoutCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = next_pointer,
        .flags = create_flags,
        .bindingCount = descriptor_count,
        .pBindings = descriptor_pairs
    };

    VkDescriptorSetLayout resulting_set_layout = {};

    vk_assert_msg(  vkCreateDescriptorSetLayout(descriptor_device, 
                                                &info, 
                                                nullptr, 
                                                &resulting_set_layout),
                    "Error creating descritpor set layout");

    return resulting_set_layout;
}


// DESCRIPTOR SET POOL ALLOCATOR
void sDSetPoolAllocator::init(  const VkDevice device, 
                                const uint32_t cmax_sets, 
                                const sDSetPoolAllocator::sPoolRatio* pool_ratios, 
                                const uint8_t ratio_count ) {
    if (first_node != nullptr) {
        clean();
    }
    
    pool_device = device;
    max_sets = cmax_sets;

    // Store the pool sizes
    for(uint32_t i = 0u; i < ratio_count; i++) {
        pool_sizes[i] = {
            .type = pool_ratios[i].type,
            .descriptorCount = max_sets * pool_ratios[i].ratio
        };
    }
    pool_sizes_count = ratio_count;

    // Init linked list node
    {
        first_node = (sLLNode*) malloc(sizeof(sLLNode));
        first_node->next_node = nullptr;
        first_node->used_pools_count = 0u;
        
        top_node = first_node;
    }

    // Create the first pool
    {
        VkDescriptorPoolCreateInfo pool_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0u,
            .maxSets = max_sets,
            .poolSizeCount = pool_sizes_count,
            .pPoolSizes = pool_sizes
        };

        vk_assert_msg(  vkCreateDescriptorPool( device, 
                                                &pool_info, 
                                                nullptr, 
                                                &top_node->pools[0]),
                        "Error creating initial descriptor pool");
    }
}

void sDSetPoolAllocator::clear_descriptors() const {
    sLLNode *it_node = first_node;

    while(it_node != nullptr) {
        for(uint16_t i = 0; i <= it_node->used_pools_count; i++) {
            vkResetDescriptorPool(pool_device, it_node->pools[i], 0u);
        }

        it_node->used_pools_count = 0u;
        it_node = it_node->next_node;
    }
}

void sDSetPoolAllocator::clean() const {
    sLLNode *it_node = first_node;
    sLLNode *prev_node = nullptr;

    while(it_node != nullptr) {
        for(uint16_t i = 0; i <= it_node->used_pools_count; i++) {
            vkDestroyDescriptorPool(pool_device, it_node->pools[i], nullptr);
        }

        prev_node = it_node;
        it_node = it_node->next_node;
        free(prev_node);
    }
}

void sDSetPoolAllocator::move_to_next_pool() {
    if (top_node->used_pools_count == DESCRIPTOR_POOLS_SIZE - 1u) {
        sLLNode *new_node = (sLLNode*) malloc(sizeof(sLLNode));
        new_node->next_node = nullptr;
        new_node->used_pools_count = 0u;

        top_node->next_node = new_node;
        top_node = new_node;

        return;
    }

    top_node->used_pools_count++;

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .maxSets = max_sets,
        .poolSizeCount = pool_sizes_count,
        .pPoolSizes = pool_sizes
    };

    vk_assert_msg(  vkCreateDescriptorPool( pool_device, 
                                            &pool_info, 
                                            nullptr, 
                                            &top_node->pools[top_node->used_pools_count]),
                    "Error creating descriptor pool");
}

VkDescriptorSet sDSetPoolAllocator::alloc(const VkDescriptorSetLayout &layout) {
    VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = top_node->pools[0],
        .descriptorSetCount = 1u,
        .pSetLayouts = &layout
    };

    VkDescriptorSet descriptor_set;

    VkResult result = vkAllocateDescriptorSets( pool_device, 
                                                &alloc_info,
                                                &descriptor_set);
    
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
        move_to_next_pool();

        alloc_info.descriptorPool = top_node->pools[top_node->used_pools_count];

        result = vkAllocateDescriptorSets(  pool_device, 
                                            &alloc_info,
                                            &descriptor_set);
    }
    vk_assert_msg(  result, 
                    "Error allocating a descriptor set (not out of memory or fragmented)");

    return descriptor_set;
}