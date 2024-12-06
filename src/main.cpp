#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "common.h"
#include "utils.h"

#include "render/renderer.h"
#include "render/vk_helpers.h"

#include "resource_manager.h"

int main() {
    spdlog::info("Initalizing render");

    Render::sBackend renderer;
    bool success = renderer.init();

    spdlog::info("Starting the render loop");
    while(!glfwWindowShouldClose(renderer.gpu_instance.window)) {
        glfwPollEvents();

        renderer.render();
    }

    spdlog::info("Cleaning the render");
    renderer.clean();

    return 0;
}