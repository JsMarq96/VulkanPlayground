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