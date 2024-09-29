#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "common.h"
#include "utils.h"

#include "render/renderer.h"
#include "render/vk_helpers.h"

int main() {
    spdlog::info("Initalizing render");

    Render::sBackend renderer;
    bool success = renderer.init();

    spdlog::info("Starting the render loop");
    while(!glfwWindowShouldClose(renderer.gpu_instance.window)) {
        glfwPollEvents();

        renderer.start_frame_capture();

        // TODO Render something
        VkClearColorValue clear_color = {{ 0.0f, 0.0f, std::abs(std::sin(renderer.frame_number / 60.f)), 1.0f  }};

        VkImageSubresourceRange clear_range = VK_Helpers::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

        vkCmdClearColorImage(renderer.get_current_frame().cmd_buffer, renderer.draw_image.image, VK_IMAGE_LAYOUT_GENERAL, &clear_color, 1u, &clear_range);

        renderer.end_frame_capture();
    }

    spdlog::info("Cleaning the render");
    renderer.clean();

    return 0;
}