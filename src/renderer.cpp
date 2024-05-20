#include "renderer.h"

#include <GLFW/glfw3.h>

#include "common.h"
#include "utils.h"

void Renderer::init_graphics_instance(Renderer::sInstance *instance) {
    // Create window
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        instance->window = glfwCreateWindow(WIN_HEIGHT, WIN_WIDTH, WIN_NAME, NULL, NULL);
        assert_msg(instance->window, "Error creating GLFW window");
    }

    // Create vulkan instance
    {
        vkb::InstanceBuilder instance_builder;
        vkb::Result<vkb::Instance> res = instance_builder
                                            .set_app_name(WIN_NAME)
                                            .require_api_version(1u, 1u, 0u)
                                            .use_default_debug_messenger()
                                            .request_validation_layers()
                                            .build();
        if (!res) {
            assert_msg(false, res.error().message());
        }
        instance->vulkan_instance = res.value();
    }

    // Create render surface
    {
        vk_assert_msg(glfwCreateWindowSurface(instance->vulkan_instance, 
                                              instance->window, 
                                              nullptr, 
                                              &(instance->surface)),
                      "Error loading GLFW surface");  
    }

    // Load physical device
    {
        vkb::PhysicalDeviceSelector phys_device_selector(instance->vulkan_instance);
        vkb::Result<vkb::PhysicalDevice> phys_device_result = phys_device_selector
                                                                .set_surface(instance->surface)
                                                                .select();
        if (!phys_device_result) {
            assert_msg(false, phys_device_result.error().message());
        }
        instance->physical_device = phys_device_result.value();
    }

    // Load Device
    {
        vkb::Result<vkb::Device> device_result = (vkb::DeviceBuilder{ instance->physical_device }).build();
        if (!device_result) {
            assert_msg(false, device_result.error().message());
        }
        instance->device = device_result.value();
        instance->dispatch_table = instance->device.make_table();
    }
}

void Renderer::render_a_frame(Renderer::sRenderData& render, const Renderer::sInstance& instance) {
    // Wait for the previus frame
    {
        vkWaitForFences(
            instance.device, 
            1u, // Fences count
            &render.in_flight_fence, 
            VK_TRUE, 
            UINT64_MAX); // Timeout (this disables it)

        // Reset the fence
        vkResetFences(instance.device, 1u, &render.in_flight_fence);
    }

    // Adquire the swapchain image
    uint32_t image_index = 0u;
    {
        // Trigger this semaphore when adquiring the image
        vkAcquireNextImageKHR(
            instance.device, 
            render.swapchain.swapchain, 
            UINT64_MAX, 
            render.semaphores.image_available,
            VK_NULL_HANDLE, 
            &image_index);
    }

    // First start recording the command buffer
    {
        // Rest the command buffer
        vkResetCommandBuffer(render.command_buffer, 0u);

        VkCommandBufferBeginInfo begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = 0u,
            .pInheritanceInfo = nullptr, // fopr secondary command buffers
        };

        vk_assert_msg(
            vkBeginCommandBuffer(
                render.command_buffer, 
                &begin_info), 
            "Error beginign the recording of teh command buffer");
    }

    // Start the render pass
    {
        VkRenderPassBeginInfo render_pass_info = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = render.render_pass,
            .framebuffer = render.framebuffers[image_index],
            .renderArea = { // Area to render in the framebuffer
                .offset = {0u, 0u},
                .extent = render.swapchain.extent
            },
            .clearValueCount = 1u,
            .pClearValues = &render.clear_color
        };

        vkCmdBeginRenderPass(
            render.command_buffer, 
            &render_pass_info, 
            VK_SUBPASS_CONTENTS_INLINE); // This comands will be sent to the prymary command
    }

    // Record render calls
    {
        vkCmdBindPipeline(
            render.command_buffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS, 
            render.graphics_pipeline);

        // Set Viewport and scissor
        VkViewport viewport = {
            .x = 0.0f, .y = 0.0f,
            .width = static_cast<float>(render.swapchain.extent.width),
            .height = static_cast<float>(render.swapchain.extent.height),
            .minDepth = 0.0f, .maxDepth = 1.0f
        };
        vkCmdSetViewport(
            render.command_buffer, 
            0u, // first viewport
            1u, // number of viewports
            &viewport);

        VkRect2D scissors = {
            .offset = {0u, 0u},
            .extent = render.swapchain.extent
        };
        vkCmdSetScissor(
            render.command_buffer, 
            0u, 
            1u, 
            &scissors);

        // Render the triangle!
        vkCmdDraw(render.command_buffer, 3u, 1u, 0u, 0u);
    }

    // Stop the render pass and the command buffer
    {
        vkCmdEndRenderPass(render.command_buffer);
        vk_assert_msg(vkEndCommandBuffer(render.command_buffer), "Error ending the record of the command buffer");
    }

    const uint32_t signal_count = 1u;
    VkSemaphore signal_semaphores[1u] = {render.semaphores.image_available};
    VkPipelineStageFlags wait_stages[1u]= {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    // Submit the command buffer
    {        
        // Indicate wich semaphores to wait to, and in wich state of the pipeline
        // In this case, we wait until the image ffor render is ready (we can do, for 
        // example, vertex processing, and wait for the semaphore so start the framgent pass)

        VkSubmitInfo submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = signal_count,
            .pWaitSemaphores = signal_semaphores,
            .pWaitDstStageMask = wait_stages,
            .commandBufferCount = 1u,
            .pCommandBuffers = &render.command_buffer,
            .signalSemaphoreCount = 1u, // Signalthe render finished sempahore on finish
            .pSignalSemaphores = &render.semaphores.render_finished
        };

        vk_assert_msg(
            vkQueueSubmit(
                render.graphics_queue, 
                1u, 
                &submit_info, 
                render.in_flight_fence), 
            "Error submiting command buffer");
    }

    // Present frame
    {
        VkPresentInfoKHR present_info = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = signal_count,
            .pWaitSemaphores = signal_semaphores,
            .swapchainCount = 1u,
            .pSwapchains = &render.swapchain.swapchain, // Set wich swapchain do you want to present
            .pImageIndices = &image_index,
            .pResults = nullptr
        };

        vkQueuePresentKHR(render.present_queue, &present_info);
    }
}

void Renderer::create_framebuffers(Renderer::sRenderData *render, const Renderer::sInstance& instance) {
    std::vector<VkImage> im_arr = render->swapchain.get_images().value();
    std::vector<VkImageView> im_views_arr = render->swapchain.get_image_views().value();

    for(uint32_t i = 0u; i < render->swapchain.image_count; i++) {
        render->swapchain_image[i] = im_arr[i];
        render->swapchain_image_views[i] = im_views_arr[i];

        VkFramebufferCreateInfo framebuffer_info = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = render->render_pass,
            .attachmentCount = 1u,
            .pAttachments = render->swapchain_image_views,
            .width = render->swapchain.extent.width,
            .height = render->swapchain.extent.height,
            .layers = 1u
        };

        vk_assert_msg(instance.dispatch_table.createFramebuffer(&framebuffer_info, 
                                                                nullptr, 
                                                                &render->framebuffers[i]), 
                      "Error creating the framebuffer");
    }

    im_arr.clear();
    im_views_arr.clear();
}

void Renderer::create_render_passes(Renderer::sRenderData *render, const Renderer::sInstance& instance) {
    // First render pass (to screen)
    {
        // Render attachment
        VkAttachmentDescription present_color_attachments = {
            .format = render->swapchain.image_format,
            .samples = VK_SAMPLE_COUNT_1_BIT, // no multisampling
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // or dont care??
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, // We are rendering a frame from scratch there is no inital alyout
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR // The output is for presenting the final render
        };

        // Subpass config (only one subpass with one color attachment aka the output vec4 layout)
        VkSubpassDescription subpass;
        VkAttachmentReference color_attachment_ref;
        {
            color_attachment_ref = {
                .attachment = 0, // for the presetn color attachment
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            };

            subpass = {
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .colorAttachmentCount = 1u,
                .pColorAttachments = &color_attachment_ref
            };
        }

        // ????
        VkSubpassDependency dependency = {
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0u,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0u,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // Send the result of the color to attachment 
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT          // to the next pass
        };

        // Create the actual render pass
        VkRenderPassCreateInfo render_pass_info = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .attachmentCount = 1u,
            .pAttachments = &present_color_attachments,
            .subpassCount = 1u,
            .pSubpasses = &subpass,
            .dependencyCount = 1u,
            .pDependencies = &dependency
        };

        vk_assert_msg(
            vkCreateRenderPass(instance.device.device, 
                               &render_pass_info, 
                               nullptr, 
                               &render->render_pass),
            "Create render pass");

    }
}

void Renderer::init_render(Renderer::sRenderData *render, const Renderer::sInstance& instance) {
    // Create Swapchains
    {
        vkb::SwapchainBuilder swap_builder{instance.physical_device, instance.device, instance.surface};
        vkb::Result<vkb::Swapchain> res = swap_builder
                                            .set_old_swapchain(render->swapchain)
                                            .use_default_format_selection()
                                            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                                            .set_desired_extent(WIN_WIDTH, WIN_HEIGHT)
                                            .set_desired_min_image_count(MAX_SWAPCHAIN_SIZE)
                                            .build();
        if (!res) {
            assert_msg(false, res.error().message());
        }
        vkb::destroy_swapchain(render->swapchain);
        render->swapchain = res.value();
    }

    // Create Render Queue
    {
        vkb::Result<VkQueue> res = instance.device.get_queue(vkb::QueueType::graphics);
        if (!res) {
            assert_msg(false, res.error().message());
        }
        render->graphics_queue = res.value();
    }

    // Create Present Queue
    {
        vkb::Result<VkQueue> res = instance.device.get_queue(vkb::QueueType::present);
        if (!res) {
            assert_msg(false, res.error().message());
        }
        render->present_queue = res.value();
    }

    create_render_passes(render, instance);

    create_framebuffers(render, instance);

    create_sync_object(render, instance);

    create_render_pipeline(render, instance, "../shaders/triangle.vert.spv", "../shaders/triangle.frag.spv");

    create_command_pool_and_buffer(render, instance);
}


void Renderer::create_shader_module(VkShaderModule *new_shader_module,
                                    const Renderer::sInstance &instance, 
                                    const char* raw_code, 
                                    const uint32_t raw_code_length) {
    VkShaderModuleCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = raw_code_length,
        .pCode = (const uint32_t*) raw_code
    };

    vk_assert_msg(instance.dispatch_table.createShaderModule(&create_info, 
                                                             nullptr, 
                                                             new_shader_module),
                  "Error creating shader module");
}

void Renderer::create_render_pipeline(Renderer::sRenderData *render, const sInstance& instance, const char* vertex_shader_dir, const char* frag_shader_dir) {
    // Load shaders & create teh sahder modules
    VkShaderModule vert_shader_module, frag_shader_module;
    {
        char *vertex_shader, *frag_shader;
        uint64_t vertex_shader_len = bin_file_open(vertex_shader_dir, &vertex_shader);
        uint64_t fragment_shader_len = bin_file_open(frag_shader_dir, &frag_shader);

        create_shader_module(&vert_shader_module, instance, vertex_shader, vertex_shader_len);
        create_shader_module(&frag_shader_module, instance, frag_shader, fragment_shader_len);
    }

    // Create shader pipelines
    VkPipelineShaderStageCreateInfo shader_stages[2u]= {};
        // Vertex stage create info
        shader_stages[0u] = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vert_shader_module,
            .pName = "main"
        };

        // Fragment stage create info
        shader_stages[1u] = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = frag_shader_module,
            .pName = "main"
        };

    VkPipelineViewportStateCreateInfo viewport_state;
    VkPipelineDynamicStateCreateInfo dynamic_state = {};
    VkViewport viewport = {};
    VkRect2D scissor = {};
    // Dynamic state, viewport and scissor
    {
        dynamic_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .dynamicStateCount = 2u,
            .pDynamicStates = render->dynamic_states
        };
        
        viewport = {
            .x = 0u, .y = 0u,
            .width = static_cast<float>(render->swapchain.extent.width),
            .height = static_cast<float>(render->swapchain.extent.height),
            .minDepth = 0.0f, .maxDepth = 1.0f
        };

        scissor = {
            .offset = {0u, 0u},
            .extent = render->swapchain.extent
        };

        // If its not dynamic, set the viewport and scirrosr structs here
        viewport_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .viewportCount = 1u,
            .pViewports = nullptr,
            .scissorCount = 1u,
            .pScissors = nullptr
        };
    }

    // Create pipeline state fixed functions
    VkPipelineVertexInputStateCreateInfo vertex_input_info;
    VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
    VkPipelineRasterizationStateCreateInfo raster_config_info;
    VkPipelineMultisampleStateCreateInfo multisample_config_info;
    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info;
    VkPipelineColorBlendAttachmentState color_blend_attachment;
    {
        // Descrives teh vertex format
        vertex_input_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .vertexBindingDescriptionCount = 0u,
            .pVertexBindingDescriptions = nullptr,
            .vertexAttributeDescriptionCount = 0u,
            .pVertexAttributeDescriptions = nullptr
        };

        // Input assembly (render primitive config)
        input_assembly_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE // ??
        };

        // Rasterizer mode
        raster_config_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .depthClampEnable = VK_FALSE, // clamp the depth values the near and far planes
            .rasterizerDiscardEnable = VK_FALSE, // Discard geometry for avodiging writing int he framebuffer
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT, // cull the backfaces
            .frontFace = VK_FRONT_FACE_CLOCKWISE, // Set what we consider a front face
            .depthBiasEnable = VK_FALSE, // add a bias to the depth buffer, and the config
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = 1.0f, // for rendering lines mode
        };

        // Multisample config
        multisample_config_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,\
            .minSampleShading = 1.0f, // ??
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };

        // Color blending
        color_blend_attachment = {
            .blendEnable = VK_FALSE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };
        color_blend_state_create_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount = 1u,
            .pAttachments = &color_blend_attachment,
            .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
        };
    }

    // Create the pipeline layout
    // For sending uniform values to the shader pipeline
    {
        VkPipelineLayoutCreateInfo pipeline_layout_create = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .setLayoutCount = 0u, // For attribute data
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 0u, // Uniforms, to be updated for each pipeline
            .pPushConstantRanges = nullptr
        };

        vk_assert_msg(vkCreatePipelineLayout(instance.device, 
                            &pipeline_layout_create, 
                            nullptr, 
                            &render->pipeline_layout),
                     "Error creating the pipeline layout");
    }

    // Create the render Pipeline
    VkGraphicsPipelineCreateInfo pipeline_create_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0u,
        .stageCount = 2u,
        .pStages = shader_stages,
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly_info,
        .pTessellationState = nullptr,
        .pViewportState = &viewport_state,
        .pRasterizationState = &raster_config_info,
        .pMultisampleState = &multisample_config_info,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &color_blend_state_create_info,
        .pDynamicState = &dynamic_state,
        .layout = render->pipeline_layout,
        .renderPass = render->render_pass,
        .subpass = 0u, // Inbdex of the subpass
        .basePipelineHandle = VK_NULL_HANDLE, //for derivating a new pipeline from another
        .basePipelineIndex = -1
    };

    vk_assert_msg(
        vkCreateGraphicsPipelines(
            instance.device, 
            VK_NULL_HANDLE, 
            1u, 
            &pipeline_create_info,
            nullptr, 
            &render->graphics_pipeline),
        "Error creating the graphics pipeline");
}


void Renderer::create_command_pool_and_buffer(Renderer::sRenderData *render, const Renderer::sInstance& instance) {
    const std::vector<VkQueueFamilyProperties> properties = instance.physical_device.get_queue_families();

    VkCommandPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // depending on the frequency of how many times the command buffer is reordered
        .queueFamilyIndex = instance.device.get_queue_index(vkb::QueueType::graphics).value()
    };

    vk_assert_msg(
        vkCreateCommandPool(
            instance.device, 
            &pool_info, 
            nullptr, 
            &render->command_pool),
        "Failed to create commnd pool");

    VkCommandBufferAllocateInfo buffer_alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = render->command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, // can be submited for execution, if secundary can only be called from prymary ones
        .commandBufferCount = 1u
    };

    vk_assert_msg(
        vkAllocateCommandBuffers(
            instance.device, 
            &buffer_alloc_info, 
            &render->command_buffer), 
        "Error allocating the command buffer");
}

void Renderer::create_sync_object(Renderer::sRenderData *render, const Renderer::sInstance& instance) {
    // Create semaphores
    {
        VkSemaphoreCreateInfo semaphore_create_info = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
        };

        vk_assert_msg(
            vkCreateSemaphore(
                instance.device, 
                &semaphore_create_info, 
                nullptr, 
                &render->semaphores.image_available),
            "Error creating semaphore for iamge available");

        vk_assert_msg(
            vkCreateSemaphore(
                instance.device, 
                &semaphore_create_info, 
                nullptr, 
                &render->semaphores.render_finished),
            "Error creating semaphore for render finished");
    }

    // Create fences
    {
        VkFenceCreateInfo fence_create_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        vk_assert_msg(
            vkCreateFence(
                instance.device, 
                &fence_create_info, 
                nullptr, 
                &render->in_flight_fence), 
            "Errore creating the in-flight fence render finished");
    }
}