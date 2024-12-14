#pragma once

#include <vulkan/vulkan.h>

#define MAX_BIDING_COUNT 8u
#define MAX_DESCRIPTOR_TYPE_COUNT 9u
#define DESCRIPTOR_POOLS_SIZE 10u

enum eDescriptorTypes : uint32_t {
    DESCRIPTOR_SAMPLER = VK_DESCRIPTOR_TYPE_SAMPLER,
    DESCRIPTOR_IMAGE_SAMPLER = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    DESCRIPTOR_SAMPLED_IMAGE = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    DESCRIPTOR_STORAGE_IMAGE = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
    DESCRIPTOR_UNIFORM_TEXEL_BUFFER = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
    DESCRIPTOR_STORAGE_TEXEL_BUFFER = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
    DESCRIPTOR_UNIFORM_BUFFER = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    DESCRIPTOR_STORAGE_BUFFER = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    DESCRIPTOR_UNIFORM_BUFFER_DYNAMIC = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
    DESCRIPTOR_STORAGE_BUFFER_DYNAMIC = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
};

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
    struct sLLNode {
        sLLNode *next_node = nullptr;
        VkDescriptorPool pools[DESCRIPTOR_POOLS_SIZE] = {};
        uint16_t used_pools_count = 0u;
    };

    struct sPoolRatio {
        VkDescriptorType    type;
        uint32_t            ratio;
    };

    VkDescriptorPoolSize  pool_sizes[MAX_DESCRIPTOR_TYPE_COUNT] = {};
    uint8_t pool_sizes_count = 0u;

    uint32_t max_sets = 0u;

    sLLNode *first_node = nullptr;
    sLLNode *top_node = nullptr;

    VkDevice pool_device;

    void init(const VkDevice device, const uint32_t max_sets, const sPoolRatio* pool_ratios, const uint8_t ratio_count);
    void clear_descriptors() const;
    void clean() const;

    void move_to_next_pool();

    VkDescriptorSet alloc(const VkDescriptorSetLayout &layout);
};