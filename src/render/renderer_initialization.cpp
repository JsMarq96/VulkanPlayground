#include "renderer.h"

#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>
#include <VkBootstrap.h>

#include "../common.h"
#include "vk_helpers.h"

bool intialize_window(Render::sDeviceInstance &instance);
bool initialize_vulkan(Render::sDeviceInstance &instance);
bool initialize_swapchain(Render::sBackend &instance);
bool initialize_command_buffers(Render::sBackend &instance);
bool initialize_sync_structs(Render::sBackend &instance);
bool initialize_memory_alloc(Render::sBackend &instance);
bool initialize_descriptors(Render::sBackend &instance);

bool Render::sBackend::init() {
    bool is_initialized = true;

    is_initialized &= intialize_window(gpu_instance);
    is_initialized &= initialize_vulkan(gpu_instance);
    is_initialized &= initialize_memory_alloc(*this);
    is_initialized &= initialize_command_buffers(*this);
    is_initialized &= initialize_sync_structs(*this);
    is_initialized &= initialize_swapchain(*this);
    is_initialized &= initialize_descriptors(*this);

    return is_initialized;
}

bool intialize_window(Render::sDeviceInstance &instance) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    instance.window = glfwCreateWindow(WIN_HEIGHT, WIN_WIDTH, WIN_NAME, nullptr, nullptr);
    
    return !instance.window;
}

bool initialize_vulkan(Render::sDeviceInstance &instance) {
    vkb::InstanceBuilder builder;

    // Create instance
    vkb::Instance vkb_instance;
    {
        vkb::Result<vkb::Instance> result =  builder
                                                .set_app_name(WIN_NAME)
                                                .use_default_debug_messenger()
                                                .request_validation_layers(true)
                                                .require_api_version(1u, 3u, 0u)
                                                .build();

        if (!result) {
            spdlog::error("Could not load instance: {}", result.error().message());
            return false;
        }
        vkb_instance = result.value();

        instance.instance = vkb_instance.instance;
        instance.debug_messenger = vkb_instance.debug_messenger;
    }

    // Create surface
    {
        glfwCreateWindowSurface(instance.instance, 
                                instance.window, 
                                nullptr, 
                                &(instance.surface));
    }

    // Load Vulkan Features
    vkb::Device vkb_device;
    {
        // 1.3 features
        VkPhysicalDeviceVulkan13Features features_13 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = nullptr,
            .synchronization2 = true,
            .dynamicRendering = true
        };

        // 1.2 features
        VkPhysicalDeviceVulkan12Features features_12 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = nullptr,
            .descriptorIndexing = true,
            .bufferDeviceAddress = true
        };

        vkb::PhysicalDeviceSelector selector { vkb_instance };

        vkb::Result<vkb::PhysicalDevice> result = selector
                    .set_minimum_version(1u, 3u)
                    .set_required_features_13(features_13)
                    .set_required_features_12(features_12)
                    .set_surface(instance.surface)
                    .select();

        if (!result) {
            spdlog::error("Could not select a physical device {}", result.error().message());
            return false;
        }

        vkb::PhysicalDevice physical_device = result.value();

        vkb::DeviceBuilder device_builder { physical_device };

        vkb::Result<vkb::Device> result_device = device_builder.build();

        if (!result_device) {
            spdlog::error("Could not create a device {}", result_device.error().message());
            return false;
        }

	    vkb_device = result_device.value();

        instance.device = vkb_device.device;
        instance.gpu = vkb_device.physical_device;
    }

    // Load Vulkan Queue
    {
        instance.graphic_queue.queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
        instance.graphic_queue.family = vkb_device.get_queue_index(vkb::QueueType::graphics).value();
    }
    
    return true;
}

bool initialize_swapchain(Render::sBackend &instance) {
    bool swapchain_success =  instance.create_swapchain(WIN_WIDTH, 
                                                        WIN_HEIGHT, 
                                                        VK_FORMAT_B8G8R8A8_UNORM,
                                                        instance.swapchain_data);
    if (!swapchain_success) {
        return false;
    }

    VkImageUsageFlags image_usages = 0u;
    image_usages = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_usages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    image_usages |= VK_IMAGE_USAGE_STORAGE_BIT;
    image_usages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VkExtent3D draw_image_extent = {
        instance.swapchain_data.extent.width, 
        instance.swapchain_data.extent.height, 
        1u
    };

    instance.draw_image = instance.create_image(VK_FORMAT_R16G16B16A16_SFLOAT,
                                                image_usages, 
                                                VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT), 
                                                draw_image_extent);

    return true;
}

bool initialize_command_buffers(Render::sBackend &instance) {
    // Create comon command pool
    VkCommandPoolCreateInfo command_pool_create_info = VK_Helpers::create_cmd_pool_info(    instance.gpu_instance.graphic_queue.family, 
                                                                                            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

    // Create a command buffer & pool for each frame in flight
    for(uint32_t i = 0u; i < FRAME_BUFFER_COUNT; i++) {
        VkResult create_pool_res = vkCreateCommandPool(
                                        instance.gpu_instance.device, 
                                        &command_pool_create_info, 
                                        nullptr, 
                                        &instance.in_flight_frames[i].cmd_pool);
        if (create_pool_res != VK_SUCCESS) {
            spdlog::error("Error creating command pool");
            return false;
        }
        
        VkCommandBufferAllocateInfo cmd_alloc_info = VK_Helpers::create_cmd_buffer_alloc_info(instance.in_flight_frames[i].cmd_pool, 1u);

        VkResult allocate_cmd_res = vkAllocateCommandBuffers(
                                        instance.gpu_instance.device, 
                                        &cmd_alloc_info, 
                                        &instance.in_flight_frames[i].cmd_buffer);

        if (allocate_cmd_res != VK_SUCCESS) {
            spdlog::error("Error allocating command buffer");
            return false;
        }
    }

    return true;
}

bool initialize_sync_structs(Render::sBackend &instance) {
    // Start fence as signaled
    VkFenceCreateInfo fence_Create_info = VK_Helpers::create_fence_info(VK_FENCE_CREATE_SIGNALED_BIT);
    VkSemaphoreCreateInfo sempahore_create_info = VK_Helpers::create_semaphore_info();
    
    for(uint32_t i = 0u; i < FRAME_BUFFER_COUNT; i++) {
        VkResult fence_res = vkCreateFence( instance.gpu_instance.device, 
                                            &fence_Create_info, 
                                            nullptr, 
                                            &instance.in_flight_frames[i].render_fence);
        if (fence_res != VK_SUCCESS) {
            spdlog::error("Error creating fence");
            return false;
        }

        VkResult sem_swap = vkCreateSemaphore(  instance.gpu_instance.device, 
                                                &sempahore_create_info, 
                                                nullptr, 
                                                &instance.in_flight_frames[i].swapchain_semaphore);
        if (sem_swap != VK_SUCCESS) {
            spdlog::error("Error creating swapchain semaphore");
            return false;
        }
        VkResult sem_ren = vkCreateSemaphore(   instance.gpu_instance.device, 
                                                &sempahore_create_info, 
                                                nullptr, 
                                                &instance.in_flight_frames[i].render_semaphore);
        if (sem_ren != VK_SUCCESS) {
            spdlog::error("Error creating render semaphore");
            return false;
        }
    }
    return true;
}

bool initialize_memory_alloc(Render::sBackend &instance) {
    VmaAllocatorCreateInfo alloc_create_info = {
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = instance.gpu_instance.gpu,
        .device = instance.gpu_instance.device,
        .instance = instance.gpu_instance.instance
    };

    vmaCreateAllocator( &alloc_create_info, 
                        &instance.vk_allocator);

    // TODO: push destroy?

    return true;
};

bool initialize_descriptors(Render::sBackend &instance) {
    return true;
}