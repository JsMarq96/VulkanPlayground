#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>

#include "resources.h"
#include "pipeline.h"

#define FRAME_BUFFER_COUNT 3u

struct GLFWwindow;

namespace Render {

    struct sComputePushConstants {
        glm::vec4   data1;
        glm::vec4   data2;
        glm::vec4   data3;
        glm::vec4   data4;
    };

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

    // https://vkguide.dev/docs/new_chapter_3/render_pipeline/
   
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
        sDeviceInstance         gpu_instance = {};

        VmaAllocator            vk_allocator;

        uint64_t                frame_number = 0u;
        sFrame                  in_flight_frames[FRAME_BUFFER_COUNT];

        sDSetPoolAllocator      global_descritpor_allocator;
        VkDescriptorSet         draw_image_descriptor_set;
        VkDescriptorSetLayout   draw_image_descritpor_layout;

        VkPipeline              gradient_draw_compute_pipeline;
        VkPipelineLayout        gradient_draw_compute_pipeline_layout;

        VkPipeline              render_triangle_pipeline;
        VkPipelineLayout        render_triangle_pipeline_layout;

        struct sSwapchainData {
            VkFormat        format;
            VkExtent2D      extent;
            VkSwapchainKHR  swapchain;
            VkImage         images[FRAME_BUFFER_COUNT];
            VkImageView     image_views[FRAME_BUFFER_COUNT];
        } swapchain_data;

        sImage              draw_image;

        bool create_swapchain(const uint32_t width, const uint32_t height, const VkFormat format, sSwapchainData &swapchain_data);
        void destroy_swapchain(sSwapchainData &swapchain_data);

        sImage create_image(const VkFormat img_format, const VkImageUsageFlags usage, const VkMemoryPropertyFlags mem_flags, const VkExtent3D& img_dims, const VkImageAspectFlagBits view_flags = VK_IMAGE_ASPECT_COLOR_BIT);

        inline sFrame& get_current_frame() { 
            return in_flight_frames[frame_number % FRAME_BUFFER_COUNT]; 
        };

        void start_frame_capture();
        void end_frame_capture();

        void render();

        bool init();
        void clean();
    };

};