#include "../renderer.h"

#include <VkBootstrap.h>
#include <spdlog/spdlog.h>
#include <stdint.h>

#include "../../utils.h"

bool Render::sBackend::create_swapchain(    const uint32_t width, 
                                            const uint32_t height, 
                                            const eImageFormats format, 
                                            sBackend::sSwapchainData &swapchain_data) {
    swapchain_data.format = (VkFormat) format;

    vkb::SwapchainBuilder swapchain_builder(
        gpu_instance.gpu, 
        gpu_instance.device, 
        gpu_instance.surface
    );

    vkb::Result<vkb::Swapchain> swapchain_result = swapchain_builder
            .set_desired_format(
                VkSurfaceFormatKHR{
                    .format = (VkFormat) format,
                    .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR 
                })
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(width, height)
            .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
            .build();

    if (!swapchain_result) {
        spdlog::error("Error creating swapchains {}", swapchain_result.error().message());
        return false;
    }

    vkb::Swapchain vkb_swapchain = swapchain_result.value();

    swapchain_data.swapchain = vkb_swapchain.swapchain;
    swapchain_data.extent = vkb_swapchain.extent;

    std::vector<VkImage> images = vkb_swapchain.get_images().value();
    std::vector<VkImageView> image_views = vkb_swapchain.get_image_views().value();

    uint32_t im_count = image_views.size();
    uint32_t im_count1 = images.size();

    assert_msg(FRAME_BUFFER_COUNT <= images.size() || FRAME_BUFFER_COUNT <= image_views.size(), "Too many images");

    memcpy(swapchain_data.images, images.data(), images.size() * sizeof(VkImage));
    memcpy(swapchain_data.image_views, image_views.data(), image_views.size() * sizeof(VkImageView));

    return true;
}

void Render::sBackend::destroy_swapchain(sBackend::sSwapchainData &swapchain_data) {
    vkDestroySwapchainKHR(gpu_instance.device, swapchain_data.swapchain, nullptr);

    for(uint32_t i = 0u; i < FRAME_BUFFER_COUNT; i++) {
        vkDestroyImageView(gpu_instance.device, swapchain_data.image_views[i], nullptr);
    }
}