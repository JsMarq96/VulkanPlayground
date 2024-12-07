#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>

#include "gpu_buffers.h"
#include "gpu_mesh.h"
#include "resources.h"
#include "pipeline.h"

#define FRAME_BUFFER_COUNT 3u
#define MAX_STAGING_BUFFER_COUNT 30u
#define MAX_STAGING_BUFFER_RESOLVE_COUNT (MAX_STAGING_BUFFER_COUNT * 4u)

#define MAX_MESH_COUNT 30u

struct GLFWwindow;

namespace Render {

    struct sComputePushConstants {
        glm::vec4   data1;
        glm::vec4   data2;
        glm::vec4   data3;
        glm::vec4   data4;
    };

    struct sStagingToResolve {
        sGPUBufferView src_buffer = {};
        sGPUBufferView dst_buffer = {};
    };

    struct sFrame {
        VkCommandPool       cmd_pool;
        VkCommandBuffer     cmd_buffer;
        // For signaling that the swapchain is being used
        VkSemaphore         swapchain_semaphore;
        // For signaling that the render has finished
        VkSemaphore         render_semaphore;

        // For waiting until the prev frame is finished
        VkFence             render_fence;
        uint32_t            current_swapchain_index = 0u;

        // Staging buffers for a frame
        uint32_t            staging_to_resolve_count = 0u;
        sStagingToResolve   staging_to_resolve[MAX_STAGING_BUFFER_RESOLVE_COUNT] = {};

        uint32_t            staging_buffer_count = 0u;
        sGPUBuffer          staging_buffers[MAX_STAGING_BUFFER_COUNT] = {};
    };

    struct sQueueData {
        VkQueue                     queue;
        uint32_t                    family;
    };

    // https://vkguide.dev/docs/new_chapter_3/mesh_buffers/
   
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

        // VkPipeline              render_triangle_pipeline;
        // VkPipelineLayout        render_triangle_pipeline_layout;

        sGPUMesh                rectangle_mesh;

        VkPipeline              render_mesh_pipeline;
        VkPipelineLayout        render_mesh_pipeline_layout;

        struct sSwapchainData {
            VkFormat        format;
            VkExtent2D      extent;
            VkSwapchainKHR  swapchain;
            VkImage         images[FRAME_BUFFER_COUNT];
            VkImageView     image_views[FRAME_BUFFER_COUNT];
        } swapchain_data;

        sImage              draw_image;

        // Renderables
        uint32_t            mesh_count = 0u;
        sGPUMesh            meshes[MAX_MESH_COUNT] = {};

        bool create_swapchain(const uint32_t width, const uint32_t height, const VkFormat format, sSwapchainData &swapchain_data);
        void destroy_swapchain(sSwapchainData &swapchain_data);

        sImage create_image(const VkFormat img_format, const VkImageUsageFlags usage, const VkMemoryPropertyFlags mem_flags, const VkExtent3D& img_dims, const VkImageAspectFlagBits view_flags = VK_IMAGE_ASPECT_COLOR_BIT);

        sGPUBuffer create_buffer(const size_t buffer_size, const VkBufferUsageFlags usage, const VmaMemoryUsage mem_usage, const bool mapped_on_startup = false);
        void clean_buffer(const sGPUBuffer &buffer);
        void upload_to_gpu(const void* data, const size_t upload_size, sGPUBuffer *dst_buffer, const size_t dst_offset, sFrame &frame_to_upload);

        bool create_gpu_mesh(Render::sGPUMesh *new_mesh, const uint32_t *indices, const uint32_t index_count, const sVertex *vertices, const uint32_t vertex_count, sFrame &frame_to_arrive);

        inline sFrame& get_current_frame() { 
            return in_flight_frames[frame_number % FRAME_BUFFER_COUNT]; 
        };

        void start_frame_capture();
        void end_frame_capture();
        void clean_prev_frame();

        void render();

        bool init();
        void clean();
    };

};