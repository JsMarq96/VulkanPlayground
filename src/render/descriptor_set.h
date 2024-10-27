#pragma once

#include <vulkan/vulkan.h>

struct sDescriptorLayoutBuilder {
    // Assemble Descriptor set layouts with the builder pattern
    VkDescriptorSetLayoutBinding descriptor_pairs[MAX_BIDING_COUNT];
    uint8_t descriptor_count = 0u;

    VkDevice descriptor_device;
    VkShaderStageFlags descriptor_shader_stage;
    VkDescriptorSetLayoutCreateFlags create_flags = 0u;
    void* p_next = nullptr;

    static sDescriptorLayoutBuilder create( const VkDevice device, 
                                            const VkShaderStageFlags shader_stage, 
                                            void* p_next = nullptr, 
                                            const VkDescriptorSetLayoutCreateFlags flags = 0u);
    sDescriptorLayoutBuilder& add_biding(   const uint8_t binding, 
                                            const VkDescriptorType type);
    VkDescriptorSetLayout build(    void *next_pointer = nullptr);
};

struct sDSetPoolAllocator {
    struct sPoolRatio {
        VkDescriptorType    type;
        uint32_t            ratio;
    };

    VkDescriptorPool pool;
    VkDevice pool_device;

    void init(const VkDevice device, const uint32_t max_sets, const sPoolRatio* pool_ratios, const uint32_t ratio_count);
    void clear_descriptors() const;
    void clean() const;

    VkDescriptorSet alloc(const VkDescriptorSetLayout &layout) const;

    void alloc(const VkDescriptorSetLayout *layouts, const uint32_t layout_count, VkDescriptorSet* prealloc_result_sets) const;
};