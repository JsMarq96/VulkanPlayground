#include "renderer.h"

#include "vk_helpers.h"
#include "render_utils.h"

// TODO: Delete
sImage Render::sBackend::create_image(  const VkFormat img_format, 
                                        const VkImageUsageFlags usage,
                                        const VkMemoryPropertyFlags mem_flags,
                                        const VkExtent3D& img_dims, 
                                        const VkImageAspectFlagBits view_flags) {
    sImage result = {
        .extent = img_dims,
        .format = img_format,
    };

    VkImageCreateInfo img_create_info = VK_Helpers::image2D_create_info(img_format, 
                                                                        usage, 
                                                                        img_dims);
    VmaAllocationCreateInfo img_alloc_info = {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
        .requiredFlags = mem_flags
    };

    vk_assert_msg(  vmaCreateImage( vk_allocator, 
                                    &img_create_info, 
                                    &img_alloc_info,
                                    &result.image, 
                                    &result.alloc, 
                                    nullptr),
                    "Error creating image");
    
    VkImageViewCreateInfo img_view_create_info = VK_Helpers::image_view2D_create_info(  img_format, 
                                                                                        result.image, 
                                                                                        view_flags );

    vk_assert_msg(  vkCreateImageView(  gpu_instance.device, 
                                        &img_view_create_info, 
                                        nullptr, 
                                        &result.image_view),
                    "Creating image view");
    
    return result;
}

Render::sGPUBuffer Render::sBackend::create_buffer( const size_t buffer_size, 
                                                    const VkBufferUsageFlags usage, 
                                                    const VmaMemoryUsage mem_usage, 
                                                    const bool mapped_on_startup    ) {
    sGPUBuffer new_buffer;

    VkBufferCreateInfo buff_create_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .size = buffer_size,
        .usage = usage
    };

    VmaAllocationCreateInfo vma_alloc_info = {
        .flags = (mapped_on_startup) ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0u,
        .usage = mem_usage,
    };

    vk_assert_msg(  vmaCreateBuffer(  vk_allocator, 
                                    &buff_create_info, 
                                    &vma_alloc_info, 
                                    &new_buffer.buffer, 
                                    &new_buffer.alloc, 
                                    &new_buffer.alloc_info),
                    "Error allocating GPU buffer");
        
    return new_buffer;
}

void Render::sBackend::clean_buffer(const Render::sGPUBuffer &buffer) {
    vmaDestroyBuffer(vk_allocator, buffer.buffer, buffer.alloc);
}

Render::sGPUMesh Render::sBackend::create_gpu_mesh( const uint32_t *indices, 
                                                    const uint32_t index_count, 
                                                    const sVertex *vertices, 
                                                    const uint32_t vertex_count ) {
    sGPUMesh new_mesh;

    const size_t vertex_buffer_size = index_count * sizeof(sVertex);
    const size_t index_buffer_size = index_count * sizeof(uint32_t);

    new_mesh.vertex_buffer = create_buffer( vertex_buffer_size, 
                                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                            VMA_MEMORY_USAGE_GPU_ONLY);
    
    new_mesh.index_buffer = create_buffer(  vertex_buffer_size, 
                                            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                            VMA_MEMORY_USAGE_GPU_ONLY);
    
    VkBufferDeviceAddressInfo device_adress_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .pNext = nullptr,
        .buffer = new_mesh.vertex_buffer.buffer
    };
    new_mesh.vertex_buffer_address = vkGetBufferDeviceAddress(gpu_instance.device, &device_adress_info);

    // Load data to a staging buffer
    // Frist create the buffer
    sGPUBuffer staging_buffer = create_buffer(  vertex_buffer_size + index_buffer_size,
                                                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                VMA_MEMORY_USAGE_CPU_ONLY,
                                                true);
    
    void* mapped_staging_bufffer = staging_buffer.alloc->GetMappedData();

    // Copy the data to the CPU side stating buffer
    memcpy(mapped_staging_bufffer, vertices, vertex_buffer_size);
    memcpy((void*)((uint32_t) mapped_staging_bufffer + vertex_buffer_size), indices, index_count);

    // TODO: continue here! idk if i like inmediate submit...

    return new_mesh;
}