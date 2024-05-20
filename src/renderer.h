#pragma once

#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include <VkBootstrap.h>

#define MAX_SWAPCHAIN_SIZE 3u
#define MAX_DYNAMIC_STATES 2u

class GLFWwindow;

namespace Renderer {
    struct sInstance {
        GLFWwindow              *window = nullptr;
        vkb::Instance           vulkan_instance;
        VkSurfaceKHR            surface = VK_NULL_HANDLE;
        vkb::PhysicalDevice     physical_device;
        vkb::Device             device;
        vkb::DispatchTable      dispatch_table;
    };

    struct sRenderData {
        VkQueue             graphics_queue;
        VkQueue             present_queue;
        // Swapchain data
        vkb::Swapchain      swapchain;
        VkImage             swapchain_image[MAX_SWAPCHAIN_SIZE];
        VkImageView         swapchain_image_views[MAX_SWAPCHAIN_SIZE];
        VkFramebuffer       framebuffers[MAX_SWAPCHAIN_SIZE];

        // Render Pass data
        VkRenderPass       render_pass;
        VkPipelineLayout   pipeline_layout;
        VkPipeline         graphics_pipeline;

        VkCommandPool      command_pool;
        VkCommandBuffer    command_buffer;
        
        struct {
            VkSemaphore    image_available;
            VkSemaphore    render_finished;
        } semaphores;
        VkFence            in_flight_fence;

        VkClearValue       clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};


        VkDynamicState dynamic_states[MAX_DYNAMIC_STATES] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
    };

    void init_graphics_instance(sInstance *instance);
    void init_render(sRenderData *render, const sInstance& instance);
    void create_render_pipeline(sRenderData *render, const sInstance& instance, const char* vertex_shader_dir, const char* frag_shader_dir);
    void create_shader_module(VkShaderModule *new_shader_module, const sInstance &instance, const char* raw_code, const uint32_t raw_code_length);
    void create_framebuffers(sRenderData *render, const sInstance& instance);
    void create_render_passes(sRenderData *render, const sInstance& instance);
    void create_command_pool_and_buffer(sRenderData* render, const sInstance& instance);
    void create_sync_object(sRenderData *render, const sInstance& instance);

    void render_a_frame(sRenderData &render, const sInstance& instance);
};

// https://github.com/charles-lunarg/vk-bootstrap/blob/99e51d782e63a623edfba9cf9a0b3598292b8e46/docs/getting_started.md
// https://github.com/charles-lunarg/vk-bootstrap/blob/main/example/triangle.cpp
// https://github.com/CU-Production/vk-bootstrap-triangle/blob/86ddf4c462497a7138e89042b7853faea52feda9/src/triangle/main.cpp#L215
// https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Rendering_and_presentation