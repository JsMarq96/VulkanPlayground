#pragma once

#include "rhi.h"
#include "rhi_backend.h"

#include <VkBootstrap.h>
#include <spdlog/spdlog.h>
#include <stdint.h>

#include "../utils.h"


bool Render::create_swapchain(sBackend* backend, sSwapchainParams &swapchain_params) {
    vkb::SwapchainBuilder swapchain_builder(
        (VkPhysicalDevice) backend->physical_device, 
        (VkDevice) backend->device, 
        (VkSurfaceKHR) swapchain_params.surface
    );

    vkb::Result<vkb::Swapchain> swapchain_result = swapchain_builder
            .set_desired_format(
                VkSurfaceFormatKHR{
                    .format = (VkFormat) swapchain_params.format,
                    .colorSpace = swapchain_params.color_space 
                })
            .set_desired_present_mode(swapchain_params.present_mode)
            .set_desired_extent(swapchain_params.height, swapchain_params.height)
            .add_image_usage_flags(swapchain_params.img_usage_bits)
            .build();

    if (!swapchain_result) {
        spdlog::error("Error creating swapchains {}", swapchain_result.error().message());
        return false;
    }

    vkb::Swapchain vkb_swapchain = swapchain_result.value();

    backend->swapchain_data.vk_swapchain = (uint64_t) vkb_swapchain.swapchain;
    backend->swapchain_data.width = vkb_swapchain.extent.width;
    backend->swapchain_data.height = vkb_swapchain.extent.height;

    std::vector<VkImage> images = vkb_swapchain.get_images().value();
    std::vector<VkImageView> image_views = vkb_swapchain.get_image_views().value();

    uint32_t im_count = image_views.size();
    uint32_t im_count1 = images.size();

    assert_msg(FRAME_BUFFER_COUNT <= images.size() || FRAME_BUFFER_COUNT <= image_views.size(), "Too many images");

    memcpy(backend->swapchain_data.vk_images, images.data(), images.size() * sizeof(VkImage));
    memcpy(backend->swapchain_data.vk_image_views, image_views.data(), image_views.size() * sizeof(VkImageView));

    return true;
}

void Render::delete_swapchain(sBackend* backend) {

}

VkImage Render::swapchain_adquire_next_img(sBackend* backend) {

}