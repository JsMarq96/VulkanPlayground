#include "../renderer.h"

#include <vk_mem_alloc.h>
#include <glm/gtc/integer.hpp>

#include "../../utils.h"
#include "../vk_helpers.h"
#include "../render_utils.h"

// TODO: Delete
sImage Render::sBackend::create_image(  const eImageFormats img_format, 
                                        const VkImageUsageFlags usage,
                                        const VkMemoryPropertyFlags mem_flags,
                                        const VkExtent3D& img_dims, 
                                        const VkImageAspectFlagBits view_flags,
                                        const bool mipmapped) {
    sImage result = {
        .dims = img_dims,
        .format = img_format,
    };

    VkImageCreateInfo img_create_info = VK_Helpers::image2D_create_info((VkFormat) img_format, 
                                                                        usage, 
                                                                        img_dims);
    if (mipmapped) {
        img_create_info.mipLevels = glm::floor(glm::log2(glm::max(img_dims.width, img_dims.height))) + 1u;
    }
    
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
    
    VkImageViewCreateInfo img_view_create_info = VK_Helpers::image_view2D_create_info(  (VkFormat) img_format, 
                                                                                        result.image, 
                                                                                        view_flags );

    vk_assert_msg(  vkCreateImageView(  gpu_instance.device, 
                                        &img_view_create_info, 
                                        nullptr, 
                                        &result.image_view),
                    "Creating image view");
    
    return result;
}

void    Render::sBackend::create_image( sImage *to_create,
                                        void *raw_img_data,
                                        const eImageFormats img_format, 
                                        const VkImageUsageFlags usage, 
                                        const VkMemoryPropertyFlags mem_flags, 
                                        const VkExtent3D& img_dims,
                                        Render::sFrame &frame_to_upload, 
                                        const bool mipmapped,
                                        const VkImageAspectFlagBits view_flags) {

    *to_create = create_image(    img_format, 
                                        usage, 
                                        mem_flags, 
                                        img_dims, 
                                        view_flags,
                                        mipmapped   );

    upload_to_gpu(  raw_img_data, 
                    img_format,
                    img_dims,
                    to_create,
                    {0u, 0u, 0u},
                    &frame_to_upload );
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

    new_buffer.size = buffer_size;

    vk_assert_msg(  vmaCreateBuffer(vk_allocator, 
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

bool Render::sBackend::create_gpu_mesh( Render::sGPUMesh *new_mesh,
                                        const uint32_t *indices, 
                                        const uint32_t index_count, 
                                        const sVertex *vertices, 
                                        const uint32_t vertex_count,
                                        sFrame *frame_to_arrive ) {
    const size_t vertex_buffer_size = vertex_count * sizeof(sVertex);
    const size_t index_buffer_size = index_count * sizeof(uint32_t);

    new_mesh->vertex_buffer = create_buffer( vertex_buffer_size, 
                                            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                            VMA_MEMORY_USAGE_GPU_ONLY);
    
    new_mesh->index_buffer = create_buffer(  index_buffer_size, 
                                            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                            VMA_MEMORY_USAGE_GPU_ONLY);
    
    VkBufferDeviceAddressInfo device_adress_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .pNext = nullptr,
        .buffer = new_mesh->vertex_buffer.buffer
    };
    new_mesh->vertex_buffer_address = vkGetBufferDeviceAddress(gpu_instance.device, &device_adress_info);

    upload_to_gpu(indices, index_buffer_size, &new_mesh->index_buffer, 0u, frame_to_arrive);
    upload_to_gpu(vertices, vertex_buffer_size, &new_mesh->vertex_buffer, 0u, frame_to_arrive);

    new_mesh->index_count = index_count;

    // TODO: error management
    return true;
}

void Render::sBackend::upload_to_gpu(   const void* data, 
                                        const size_t upload_size, 
                                        sGPUBuffer *gpu_dst_buffer, 
                                        const size_t dst_offset, 
                                        sFrame *frame_to_upload ) {
    assert_msg( frame_to_upload->staging_buffer_count < MAX_STAGING_BUFFER_COUNT, 
                "Too many staging buffers in a frame!");

    uint32_t staging_idx = frame_to_upload->staging_buffer_count++;
    frame_to_upload->staging_buffers[staging_idx] = create_buffer(  upload_size,
                                                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                    VMA_MEMORY_USAGE_CPU_ONLY,
                                                                    true    );

    // Get the mapped address of teh buffer on the CPU
    void* mapped_staging_buffer;
    vmaMapMemory(vk_allocator, frame_to_upload->staging_buffers[staging_idx].alloc, &mapped_staging_buffer);

    // Copy the data to the CPU side mapped stating buffer
    memcpy(mapped_staging_buffer, data, upload_size);

    vmaUnmapMemory(vk_allocator, frame_to_upload->staging_buffers[staging_idx].alloc);

    // Add to the list of the resolves, for when whe have the cmd buffer started n running
    frame_to_upload->staging_to_resolve[frame_to_upload->staging_to_resolve_count++] = {
        .src_buffer = {
            .raw_buffer = &frame_to_upload->staging_buffers[staging_idx],
            .offset = 0u,
            .size = upload_size
        },
        .dst_buffer = {
            .raw_buffer = gpu_dst_buffer,
            .offset = dst_offset,
            .size = upload_size
        }
    };
}

void Render::sBackend::upload_to_gpu(   const void* data, 
                                        const eImageFormats tex_format, 
                                        const VkExtent3D src_img_size, 
                                        sImage *gpu_dst_image, 
                                        const VkExtent3D dst_pos, 
                                        sFrame *frame_to_upload ) {
    assert_msg( frame_to_upload->staging_buffer_count < MAX_STAGING_BUFFER_COUNT, 
                "Too many staging buffers in a frame!");

    const uint32_t upload_size = get_pixel_size(tex_format) * src_img_size.width * src_img_size.height * src_img_size.depth;
    
    uint32_t staging_idx = frame_to_upload->staging_buffer_count++;
    frame_to_upload->staging_buffers[staging_idx] = create_buffer(  upload_size,
                                                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                                    VMA_MEMORY_USAGE_CPU_ONLY,
                                                                    true    );

    // Get the mapped address of teh buffer on the CPU
    void* mapped_staging_buffer;
    vmaMapMemory(vk_allocator, frame_to_upload->staging_buffers[staging_idx].alloc, &mapped_staging_buffer);

    // Copy the data to the CPU side mapped stating buffer
    memcpy(mapped_staging_buffer, data, upload_size);

    vmaUnmapMemory(vk_allocator, frame_to_upload->staging_buffers[staging_idx].alloc);

    // Add to the list of the resolves, for when whe have the cmd buffer started n running
    frame_to_upload->staging_to_resolve[frame_to_upload->staging_to_resolve_count++] = {
        .dst_is_image = true,
        .src_buffer = {
            .raw_buffer = &frame_to_upload->staging_buffers[staging_idx],
            .offset = 0u,
            .size = upload_size
        },
        .dst_image = gpu_dst_image,
        .dst_copy_pos = dst_pos,
        .img_size = src_img_size
    };
}