#include "descriptor_set.h"

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

VkDescriptorSetLayout sDescriptorLayoutBuilder::build(  void *next_pointer = nullptr ) {
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
                                const uint32_t max_sets, 
                                const sDSetPoolAllocator::sPoolRatio* pool_ratios, 
                                const uint32_t ratio_count ) {
    pool_device = device;
    VkDescriptorPoolSize *pool_size = (VkDescriptorPoolSize*) malloc(ratio_count * sizeof(VkDescriptorPoolSize));
    
    for(uint32_t i = 0u; i < ratio_count; i++) {
        pool_size[i] = {
            .type = pool_ratios[i].type,
            .descriptorCount = max_sets * pool_ratios[i].ratio
        };
    }

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .maxSets = max_sets,
        .poolSizeCount = ratio_count,
        .pPoolSizes = pool_size
    };

    vk_assert_msg(  vkCreateDescriptorPool( device, 
                                            &pool_info, 
                                            nullptr, 
                                            &pool),
                    "Error creating descriptor pool");

    free(pool_size);
}

void sDSetPoolAllocator::clear_descriptors() const {
    vkResetDescriptorPool(pool_device, pool, 0u);
}

void sDSetPoolAllocator::clean() const {
    vkDestroyDescriptorPool(pool_device, pool, nullptr);
}

VkDescriptorSet sDSetPoolAllocator::alloc(const VkDescriptorSetLayout &layout) const {
    VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = pool,
        .descriptorSetCount = 1u,
        .pSetLayouts = &layout
    };

    VkDescriptorSet descriptor_set;

    vk_assert_msg(  vkAllocateDescriptorSets(   pool_device, 
                                                &alloc_info,
                                                &descriptor_set), 
                    "Error allocating a descriptorset");

    return descriptor_set;
}

void sDSetPoolAllocator::alloc( const VkDescriptorSetLayout *layouts, 
                                const uint32_t layout_count, 
                                VkDescriptorSet* prealloc_result_sets) const {
    VkDescriptorSetAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = pool,
        .descriptorSetCount = layout_count,
        .pSetLayouts = layouts
    };

    vk_assert_msg(  vkAllocateDescriptorSets(   pool_device, 
                                                &alloc_info,
                                                prealloc_result_sets), 
                    "Error allocating multiple descriptorsets");
}