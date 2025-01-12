#include "../renderer.h"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "../vk_helpers.h"
#include "../gpu_mesh.h"

void clear_screen(Render::sBackend &renderer);
void render_background(Render::sBackend &renderer);
void render_geometry(Render::sBackend &renderer);

void Render::sBackend::render() {
    start_frame_capture();
    
    clear_screen(*this);
    render_background(*this);
    render_geometry(*this);

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

void render_geometry(Render::sBackend &renderer) {
    Render::sFrame &current_frame = renderer.get_current_frame();

    VK_Helpers::transition_image_layout(current_frame.cmd_buffer,
                                        renderer.draw_image.image, 
                                        VK_IMAGE_LAYOUT_GENERAL, 
                                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    
    VkRenderingAttachmentInfo color_attachment_info = VK_Helpers::attachment_info(  renderer.draw_image.image_view, 
                                                                                    nullptr, 
                                                                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    VkRenderingAttachmentInfo depth_attachment_info = VK_Helpers::depth_attachment_create_info( renderer.depth_image.image_view, 
                                                                                                VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
    
    VkRenderingInfo render_info = VK_Helpers::create_render_info(   renderer.swapchain_data.extent, 
                                                                    &color_attachment_info, 
                                                                    &depth_attachment_info  );

    vkCmdBeginRendering(current_frame.cmd_buffer, &render_info);

    vkCmdBindPipeline(current_frame.cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer.render_mesh_pipeline);

    // Set viewport
    {
        VkViewport viewport = {
            .x = 0u, .y = 0u,
            .width = (float) renderer.swapchain_data.extent.width,
            .height = (float) renderer.swapchain_data.extent.height,
            .minDepth = 0.0f, 
            .maxDepth = 1.0f
        };

        vkCmdSetViewport(current_frame.cmd_buffer, 0u, 1u, &viewport);
    }

    // Set render scissor
    {
        VkRect2D scissor = {
            .offset = {.x = 0u, .y = 0u},
            .extent = {
                .width = renderer.swapchain_data.extent.width,
                .height = renderer.swapchain_data.extent.height
            }
        };

        vkCmdSetScissor(current_frame.cmd_buffer, 0u, 1u, &scissor);
    }

    vkCmdBindDescriptorSets(current_frame.cmd_buffer, 
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderer.render_mesh_pipeline_layout,
                            0u,
                            1u,
                            &current_frame.gpu_comon_scene_descriptor_set,
                            0u,
                            nullptr);
    
    for(uint32_t i = 0u; i < renderer.mesh_count; i++) {
        const Render::sGPUMesh &curr_mesh = renderer.meshes[i];

        Render::sMeshPushConstant push_constants = {
            .mvp_matrix = glm::translate(glm::vec3{ -2.0f + (2.0f * i), 0.0f, 0.0f }),
            .vertex_buffer = curr_mesh.vertex_buffer_address
        };

        vkCmdPushConstants(current_frame.cmd_buffer, renderer.render_mesh_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0u, sizeof(Render::sMeshPushConstant), &push_constants);
        vkCmdBindIndexBuffer(current_frame.cmd_buffer, curr_mesh.index_buffer.buffer, 0u, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(current_frame.cmd_buffer, curr_mesh.index_count, 1u, 0u, 0u, 0u);
    }

    vkCmdEndRendering(current_frame.cmd_buffer);
}