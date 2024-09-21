#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>

#include "common.h"
#include "utils.h"

#include "render/renderer.h"

int main() {
    spdlog::info("Initalizing render");

    Render::sBackend renderer;
    bool success = renderer.init();

    spdlog::info("Starting the render loop");
    while(!glfwWindowShouldClose(renderer.gpu_instance.window)) {
        glfwPollEvents();

        renderer.start_frame_capture();

        // TODO Render geo

        renderer.end_frame_capture();
    }

    spdlog::info("Cleaning the render");
    renderer.clean();

    return 0;
}