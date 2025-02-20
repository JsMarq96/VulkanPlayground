#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

enum eImageFormats : uint32_t {
    IMG_FORMAT_RGBA_8BIT_UNORM = VK_FORMAT_R8G8B8A8_UNORM,
    IMG_FORMAT_BRGA_8BIT_UNORM = VK_FORMAT_B8G8R8A8_UNORM,
    IMG_FORMAT_RGBA_8BIT_UINT = VK_FORMAT_R8G8B8A8_UINT,
    IMG_FORMAT_RGB_8BIT_UINT = VK_FORMAT_R8G8B8_UINT,
    IMG_FORMAT_R_8BIT_UINT = VK_FORMAT_R8_UINT,
    IMG_FORMAT_R_16BIT_SFLOAT = VK_FORMAT_R16_SFLOAT,
    IMG_FORMAT_R_32BIT_SFLOAT = VK_FORMAT_R32_SFLOAT,
    IMG_FORMAT_D_32BIT_SFLOAT = VK_FORMAT_D32_SFLOAT,
    IMG_FORMAT_RGBA_32BIT_SFLOAT = VK_FORMAT_R32G32B32A32_SFLOAT,
    IMG_FORMAT_RGBA_16BIT_SFLOAT = VK_FORMAT_R16G16B16A16_SFLOAT,
    IMG_FORMAT_UNDEF = VK_FORMAT_UNDEFINED
};
