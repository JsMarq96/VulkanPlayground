#pragma once

#include <vulkan/vulkan.h>

#define FRAME_BUFFER_COUNT 3u

struct GLFWwindow;

namespace Render {

    struct sFrame {
        VkCommandPool       cmd_pool;
        VkCommandBuffer     cmd_buffer;
        // For signaling that the swapchain is being used
        VkSemaphore         swapchain_semaphore;
        // For signaling that the render has finished
        VkSemaphore         render_semaphore;

        // Wait for waiting until the prev frame is finished
        VkFence             render_fence;
        uint32_t            current_swapchain_index = 0u;
    };

    struct sQueueData {
        VkQueue                     queue;
        uint32_t                    family;
    };

    // https://vkguide.dev/docs/new_chapter_1/vulkan_mainloop_code/
    // 	VK_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submit, get_current_frame()._renderFence));

    struct sDeviceInstance {
        GLFWwindow                  *window = nullptr;
        VkInstance                  instance;
        VkDebugUtilsMessengerEXT    debug_messenger;
        VkPhysicalDevice            gpu;
        VkDevice                    device;
        VkSurfaceKHR                surface;
        sQueueData                  graphic_queue;
    };

    struct sBackend {
        sDeviceInstance     gpu_instance = {};

        uint64_t            frame_number = 0u;
        sFrame              in_flight_frames[FRAME_BUFFER_COUNT];

        struct sSwapchainData {
            VkFormat        format;
            VkExtent2D      extent;
            VkSwapchainKHR  swapchain;
            VkImage         images[FRAME_BUFFER_COUNT];
            VkImageView     image_views[FRAME_BUFFER_COUNT];
        } swapchain_data;

        bool create_swapchain(const uint32_t width, const uint32_t height, const VkFormat format, sSwapchainData &swapchain_data);
        void destroy_swapchain(sSwapchainData &swapchain_data);

        inline sFrame& get_current_frame() { 
            return in_flight_frames[frame_number % FRAME_BUFFER_COUNT]; 
        };

        void start_frame_capture();
        void end_frame_capture();

        bool init();
        void clean();
    };

};