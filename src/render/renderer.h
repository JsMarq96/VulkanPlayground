#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>

#include "resources/descriptor_set.h"
#include "resources/gpu_buffers.h"
#include "resources/gpu_mesh.h"
#include "resources/resources.h"
#include "resources/pipeline.h"

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
        bool dst_is_image = false;
        sGPUBufferView src_buffer = {};
        sGPUBufferView dst_buffer = {};

        sImage *dst_image = nullptr;
        VkExtent3D dst_copy_pos = {};
        VkExtent3D img_size = {};
    };

    struct sGPUSceneGlobalData {
        glm::mat4   view = {};
        glm::mat4   proj = {};
        glm::mat4   view_proj = {};
        glm::vec4   ambient_color = {};
        glm::vec3   sunlight_dir = {};
        float       sun_power = 0.0f;
        glm::vec4   sunlight_color = {0.0f, 0.0f, 0.0f, 0.0f};
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

        uint32_t            staging_to_clean_count = 0u;
        sGPUBuffer          staging_to_clean[MAX_STAGING_BUFFER_COUNT] = {};

        // In-frame descriptor sets
        sDSetPoolAllocator  descriptor_allocator = {};

        sGPUSceneGlobalData scene_data;

        sGPUBuffer              gpu_comon_scene_data_buffer;
        VkDescriptorSet         gpu_comon_scene_descriptor_set;
    };

    

    // https://vkguide.dev/docs/new_chapter_4/textures/
    // https://vkguide.dev/docs/new_chapter_3/resizing_window/
    // TODO: window resizing

    struct sBackend {

        struct sDeviceInstance {
            GLFWwindow                  *window = nullptr;

            VkInstance                  instance;

            VkDebugUtilsMessengerEXT    debug_messenger;

            VkPhysicalDevice            gpu;
            VkDevice                    device;
            VkSurfaceKHR                surface;

            struct sQueueData {
                VkQueue                 queue;
                uint32_t                family;
            } graphic_queue;
        } gpu_instance = {};

        VmaAllocator            vk_allocator;

        uint64_t                frame_number = 0u;
        sFrame                  in_flight_frames[FRAME_BUFFER_COUNT];

        sDSetPoolAllocator      global_descriptor_allocator = {};

        VkDescriptorSet         draw_image_descriptor_set;
        VkDescriptorSetLayout   draw_image_descritpor_layout;

        VkPipeline              gradient_draw_compute_pipeline;
        VkPipelineLayout        gradient_draw_compute_pipeline_layout;

        VkPipeline              render_mesh_pipeline;
        VkPipelineLayout        render_mesh_pipeline_layout;

        struct sSwapchainData {
            VkFormat        format;
            VkExtent2D      extent;
            VkSwapchainKHR  swapchain;
            VkImage         images[FRAME_BUFFER_COUNT];
            VkImageView     image_views[FRAME_BUFFER_COUNT];
        } swapchain_data;

        sImage                  draw_image;
        sImage                  depth_image;

        // Scene data
        sGPUSceneGlobalData     scene_global_data;
        VkDescriptorSetLayout   gpu_comon_scene_data_descriptor_set_layout;

        // Renderables
        uint32_t            mesh_count = 0u;
        sGPUMesh            meshes[MAX_MESH_COUNT] = {};

        // Scene textures
        VkSampler           nearest_sampler;
        VkSampler           filter_sampler;

        sImage              checkerboard_texture = { };

        bool create_swapchain(const uint32_t width, const uint32_t height, const eImageFormats format, sSwapchainData &swapchain_data);
        void destroy_swapchain(sBackend::sSwapchainData &swapchain_data);

        sImage create_image(const eImageFormats img_format, const VkImageUsageFlags usage, const VkMemoryPropertyFlags mem_flags, const VkExtent3D& img_dims, const VkImageAspectFlagBits view_flags = VK_IMAGE_ASPECT_COLOR_BIT, const bool mipmapped = true);
        void create_image(sImage *new_img, void *raw_img_data, const eImageFormats img_format, const VkImageUsageFlags usage, const VkMemoryPropertyFlags mem_flags, const VkExtent3D& img_dims, sFrame &frame_to_upload, const bool mipmapped = true, const VkImageAspectFlagBits view_flags = VK_IMAGE_ASPECT_COLOR_BIT);
        void clean_image(const sImage &image);

        sGPUBuffer create_buffer(const size_t buffer_size, const VkBufferUsageFlags usage, const VmaMemoryUsage mem_usage, const bool mapped_on_startup = false);
        void clean_buffer(const sGPUBuffer &buffer);
        void upload_to_gpu(const void* data, const size_t upload_size, sGPUBuffer *dst_buffer, const size_t dst_offset, sFrame *frame_to_upload);
        void upload_to_gpu(const void* data, const eImageFormats tex_format, const VkExtent3D src_img_size, sImage *dst_buffer, const VkExtent3D dst_pos, sFrame *frame_to_upload);

        bool create_gpu_mesh(Render::sGPUMesh *new_mesh, const uint32_t *indices, const uint32_t index_count, const sVertex *vertices, const uint32_t vertex_count, sFrame *frame_to_arrive);

        inline sFrame& get_current_frame() { 
            return in_flight_frames[frame_number % FRAME_BUFFER_COUNT]; 
        };

        void start_frame_capture();
        void end_frame_capture();
        void clean_prev_staging_buffers(  Render::sFrame &current_frame  );

        void render();

        bool init();
        void clean();
    };

};