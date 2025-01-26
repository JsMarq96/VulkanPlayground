#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL

#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "common.h"
#include "utils.h"

#include "render/resources/camera.h"
#include "render/renderer.h"
#include "render/vk_helpers.h"

#include "resource_manager.h"

int main() {
    spdlog::info("Initalizing render");

    Render::sBackend renderer;
    //bool success = 
    renderer.init();

    sCamera camera = {};
    camera.config_view(glm::vec3(6.0f, 0.0f, 6.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    camera.config_projection(glm::radians(45.f), (float) renderer.swapchain_data.extent.width / (float)renderer.swapchain_data.extent.height, 0.1f, 10.0f);
    //camera.config_oblique_projection(glm::vec4(0.4f, 0.0f, 0.0f, 1.0f), glm::radians(45.f), (float) renderer.swapchain_data.extent.width / (float)renderer.swapchain_data.extent.height, 0.1f, 10.0f);

    // Compute view & projection matrix
    renderer.scene_global_data.view = camera.view_mat;
    renderer.scene_global_data.proj = camera.proj_mat;
    renderer.scene_global_data.view_proj = camera.view_proj_mat;

    spdlog::info("Starting the render loop");
    while(!glfwWindowShouldClose(renderer.gpu_instance.window)) {
        glfwPollEvents();
        if (glfwGetWindowAttrib(renderer.gpu_instance.window, GLFW_ICONIFIED) != 0) {
            continue;
        }

        renderer.scene_global_data.sun_power += 0.05f;

        renderer.render();
    }

    spdlog::info("Cleaning the render");
    renderer.clean();

    return 0;
}