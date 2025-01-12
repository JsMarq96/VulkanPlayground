#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include "image_formats.h"

inline size_t get_pixel_size(const eImageFormats format) {
    switch (format) {
        case IMG_FORMAT_BRGA_8BIT_UNORM:
        case IMG_FORMAT_RGBA_8BIT_UINT:
            return sizeof(uint8_t) * 4u;
            break;
        case IMG_FORMAT_RGB_8BIT_UINT:
            return sizeof(uint8_t) * 3u;
            break;
        case IMG_FORMAT_R_8BIT_UINT:
            return sizeof(uint8_t);
            break;
        case IMG_FORMAT_R_16BIT_SFLOAT:
            return sizeof(uint16_t);
            break;
        case IMG_FORMAT_D_32BIT_SFLOAT:
        case IMG_FORMAT_R_32BIT_SFLOAT:
            return sizeof(uint32_t);
            break;
        case IMG_FORMAT_RGBA_16BIT_SFLOAT:
            return sizeof(uint16_t) * 4u;
            break;
        case IMG_FORMAT_RGBA_32BIT_SFLOAT:
            return sizeof(uint32_t) * 4u;
            break;
        default:
            return 0u;
    }
};

struct sImage {
    VkImage         image;
    VkImageView     image_view;
    VmaAllocation   alloc;
    VkExtent3D      dims;
    eImageFormats   format = IMG_FORMAT_UNDEF;
    uint32_t        mip_levels = 0u;
};