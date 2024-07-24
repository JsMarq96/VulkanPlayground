#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>

#include "common.h"
#include "utils.h"

#include "render/renderer.h"

int main() {
    spdlog::info("Test");

    Render::sInstance renderer;
    bool success = renderer.init();

    while(!glfwWindowShouldClose(renderer.gpu_instance.window)) {
        glfwPollEvents();
    }

    renderer.clean();

    return 0;
}