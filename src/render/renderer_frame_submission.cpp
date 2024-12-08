#include "renderer.h"

#include <VkBootstrap.h>
#include <spdlog/spdlog.h>
#include <stdint.h>

#include "../utils.h"
#include "render_utils.h"
#include "vk_helpers.h"

#define VK_TIMEOUT 10000000u

void resolve_staging_buffers(Render::sBackend &instance, Render::sFrame &current_frame);

void Render::sBackend::start_frame_capture() {
    sFrame &current_frame = get_current_frame();
    // Wait until the last frame has finished rendering, and reset the fence
    vkWaitForFences(gpu_instance.device, 
                    1u, 
                    &current_frame.render_fence, 
                    true, 
                    VK_TIMEOUT);

    vkResetFences(  gpu_instance.device, 
                    1u, 
                    &current_frame.render_fence );

    // Delete from prev frame

    // Get the current swapchain
    vk_assert_msg(vkAcquireNextImageKHR(gpu_instance.device, 
                                        swapchain_data.swapchain, 
                                        VK_TIMEOUT, 
                                        current_frame.swapchain_semaphore, 
                                        nullptr, 
                                        &current_frame.current_swapchain_index),
                "Error adquiring swapchain image");

    // Begin the command recording
    // Clean the cmd buffer of the last frame
    vk_assert_msg(  vkResetCommandBuffer(current_frame.cmd_buffer, 0u), 
                    "Error reseting the command buffer");
    // TODO: Might need to return this flag, if we are in VR (reuse the comand buffer for each eye)
    VkCommandBufferBeginInfo cmd_begin = VK_Helpers::create_cmd_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    vk_assert_msg(  vkBeginCommandBuffer(current_frame.cmd_buffer, &cmd_begin), 
                    "Error initializing the command buffer");

    const uint32_t swapchain_idx = current_frame.current_swapchain_index;

    resolve_staging_buffers(*this, current_frame);

    // Clean the previous frame's staging buffers
    clean_prev_staging_buffers(current_frame);

    // Set the swapchain into general mode
    // TODO: check other iamge layouts, more effectives for rendering
    VK_Helpers::transition_image_layout(current_frame.cmd_buffer, 
                                        swapchain_data.images[swapchain_idx], 
                                        VK_IMAGE_LAYOUT_UNDEFINED, 
                                        VK_IMAGE_LAYOUT_GENERAL);

    VK_Helpers::transition_image_layout(current_frame.cmd_buffer, 
                                        draw_image.image, 
                                        VK_IMAGE_LAYOUT_UNDEFINED, 
                                        VK_IMAGE_LAYOUT_GENERAL);
                                        
    VK_Helpers::transition_image_layout(current_frame.cmd_buffer, 
                                        depth_image.image, 
                                        VK_IMAGE_LAYOUT_UNDEFINED, 
                                        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
}


void Render::sBackend::end_frame_capture() {
    sFrame &current_frame = get_current_frame();

    spdlog::info(" present idx {}", current_frame.current_swapchain_index);

    VK_Helpers::transition_image_layout(current_frame.cmd_buffer,
                                        draw_image.image, 
                                        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
                                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    VK_Helpers::transition_image_layout(current_frame.cmd_buffer,
                                        swapchain_data.images[current_frame.current_swapchain_index], 
                                        VK_IMAGE_LAYOUT_UNDEFINED, 
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VK_Helpers::copy_image_image(   current_frame.cmd_buffer, 
                                    draw_image.image, 
                                    { draw_image.extent.width, draw_image.extent.height, 1 }, 
                                    swapchain_data.images[current_frame.current_swapchain_index], 
                                    { swapchain_data.extent.width, swapchain_data.extent.height, 1 });

    // Transformt the swapchain to renderable
    VK_Helpers::transition_image_layout(current_frame.cmd_buffer,
                                        swapchain_data.images[current_frame.current_swapchain_index], 
                                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    
    vk_assert_msg(  vkEndCommandBuffer(current_frame.cmd_buffer), 
                    "Error closing the command buffer");

    // Prepare submission
    VkCommandBufferSubmitInfo cmd_info = VK_Helpers::create_cmd_buffer_submit_info(current_frame.cmd_buffer);
    VkSemaphoreSubmitInfo wait_info = VK_Helpers::create_submit_semphore_info(  VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, 
                                                                                current_frame.swapchain_semaphore);
    VkSemaphoreSubmitInfo signal_info = VK_Helpers::create_submit_semphore_info(    VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, 
                                                                                    current_frame.render_semaphore);
    VkSubmitInfo2 submit = VK_Helpers::create_cmd_submit(   &cmd_info, 
                                                            &signal_info, 
                                                            &wait_info  );

    vk_assert_msg(  vkQueueSubmit2( gpu_instance.graphic_queue.queue, 
                                    1u, 
                                    &submit, 
                                    current_frame.render_fence  ), 
                    "Error Submiting de command buffer" );

    // Present frame
    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1u,
        .pWaitSemaphores = &current_frame.render_semaphore,
        .swapchainCount = 1u,
        .pSwapchains = &swapchain_data.swapchain,
        .pImageIndices = &current_frame.current_swapchain_index
    };

    vk_assert_msg( vkQueuePresentKHR(   gpu_instance.graphic_queue.queue, 
                                        &present_info), 
                    "Error presenting the swapchain");

    frame_number++;
}

void Render::sBackend::clean_prev_staging_buffers(  Render::sFrame &current_frame  ) {
    for(uint32_t i = 0u; i < current_frame.staging_to_clean_count; i++) {
        clean_buffer(current_frame.staging_to_clean[i]);
    }
}

void resolve_staging_buffers(   Render::sBackend &instance, 
                                Render::sFrame &current_frame) {
    // TODO: group calls by staging buffer
    // TODO: use just a big staging buffer, per frame, and only delete it if there is an increase in storage size
    for(uint32_t i = 0u; i < current_frame.staging_to_resolve_count; i++) {
        Render::sStagingToResolve &to_resolve = current_frame.staging_to_resolve[i];

        VkBufferCopy region = {
            .srcOffset = to_resolve.src_buffer.offset,
            .dstOffset = to_resolve.dst_buffer.offset,
            .size = to_resolve.src_buffer.size,
        };

        vkCmdCopyBuffer(current_frame.cmd_buffer, 
                        to_resolve.src_buffer.raw_buffer->buffer, 
                        to_resolve.dst_buffer.raw_buffer->buffer, 
                        1u, 
                        &region);
    }

    for(uint32_t i = 0u; i < current_frame.staging_buffer_count; i++) {
        current_frame.staging_to_clean[i] = current_frame.staging_buffers[i];     
    }

    current_frame.staging_to_clean_count = current_frame.staging_buffer_count;
    current_frame.staging_buffer_count = 0u;
    current_frame.staging_to_resolve_count = 0u;
}