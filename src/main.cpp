#include <spdlog/spdlog.h>

#include "renderer.h"
#include "utils.h"

int main() {
    spdlog::info("Test");

    Renderer::sInstance instance;
    Renderer::sRenderData render;

    Renderer::init_graphics_instance(&instance);

    Renderer::init_render(&render, instance);

    while(!glfwWindowShouldClose(instance.window)) {
        glfwPollEvents();
        Renderer::render_a_frame(render, instance);
    }

    vkDeviceWaitIdle(instance.device);

    return 0;
}