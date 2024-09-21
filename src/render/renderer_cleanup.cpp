#include "renderer.h"

#include <VkBootstrap.h>

void Render::sBackend::clean() {
    for(uint32_t i = 0u; i < FRAME_BUFFER_COUNT; i++) {
        vkDestroyCommandPool(gpu_instance.device, in_flight_frames[i].cmd_pool, nullptr);

        vkDestroyFence(gpu_instance.device, in_flight_frames[i].render_fence, nullptr);
        vkDestroySemaphore(gpu_instance.device, in_flight_frames[i].render_semaphore, nullptr);
        vkDestroySemaphore(gpu_instance.device, in_flight_frames[i].swapchain_semaphore, nullptr);
    }

    destroy_swapchain(swapchain_data);

    vkDestroySurfaceKHR(gpu_instance.instance, gpu_instance.surface, nullptr);
    vkDestroyDevice(gpu_instance.device, nullptr);

    vkb::destroy_debug_utils_messenger(gpu_instance.instance, gpu_instance.debug_messenger);
    vkDestroyInstance(gpu_instance.instance, nullptr);

    // TODO destroy window

    vkDeviceWaitIdle(gpu_instance.device);
}