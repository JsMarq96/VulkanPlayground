#include "renderer.h"

#include <cmath>

#include "vk_helpers.h"

void clear_screen(Render::sBackend &renderer);
void render_background(Render::sBackend &renderer);

void Render::sBackend::render() {
    start_frame_capture();
    
    clear_screen(*this);
    render_background(*this);

    end_frame_capture();
}

void clear_screen(Render::sBackend &renderer) {
    VkClearColorValue clear_color = {{ 0.0f, 0.0f, std::abs(std::sin(renderer.frame_number / 60.f)), 1.0f  }};

    VkImageSubresourceRange clear_range = VK_Helpers::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

    vkCmdClearColorImage(   renderer.get_current_frame().cmd_buffer, 
                            renderer.draw_image.image, 
                            VK_IMAGE_LAYOUT_GENERAL, 
                            &clear_color, 
                            1u,
                            &clear_range);
}

void render_background(Render::sBackend &renderer) {
    Render::sFrame &current_frame = renderer.get_current_frame();

    vkCmdBindPipeline(  current_frame.cmd_buffer, 
                        VK_PIPELINE_BIND_POINT_COMPUTE,
                        renderer.gradient_draw_compute_pipeline);
    
    vkCmdBindDescriptorSets(current_frame.cmd_buffer, 
                            VK_PIPELINE_BIND_POINT_COMPUTE,
                            renderer.gradient_draw_compute_pipeline_layout,
                            0u,
                            1u,
                            &renderer.draw_image_descriptor_set,
                            0u,
                            nullptr);

    // Set up push constants
    Render::sComputePushConstants push_constants;
    push_constants.data1 = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    push_constants.data2 = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

    vkCmdPushConstants( current_frame.cmd_buffer, 
                        renderer.gradient_draw_compute_pipeline_layout, 
                        VK_SHADER_STAGE_COMPUTE_BIT, 
                        0u, 
                        sizeof(Render::sComputePushConstants), 
                        &push_constants);

    const VkExtent2D &swapchain_extent = renderer.swapchain_data.extent;
    vkCmdDispatch(  current_frame.cmd_buffer, 
                    ceil(swapchain_extent.width / 16.0), 
                    ceil(swapchain_extent.height / 16.0), 
                    1u );
}