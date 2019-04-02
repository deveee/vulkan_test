//    Vulkan test - Simple Vulkan renderer
//    Copyright (C) 2019 Dawid Gan <deveee@gmail.com>
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "camera.hpp"
#include "file_manager.hpp"
#include "renderer.hpp"

#include <array>

Renderer* Renderer::m_renderer = nullptr;

Renderer::Renderer()
{
    m_renderer = this;

    m_vulkan_context = VulkanContext::getVulkanContext();
    m_vulkan_device = m_vulkan_context->getDevice();

    m_render_pass = VK_NULL_HANDLE;
    m_pipeline_layout = VK_NULL_HANDLE;
    m_graphics_pipeline = VK_NULL_HANDLE;
    m_descriptor_pool = VK_NULL_HANDLE;
    m_descriptor_set_layout = VK_NULL_HANDLE;
}

Renderer::~Renderer()
{
    vkDestroyDescriptorSetLayout(m_vulkan_device, m_descriptor_set_layout, nullptr);
    vkDestroyDescriptorPool(m_vulkan_device, m_descriptor_pool, nullptr);

    for (auto uniform_buffer : m_uniform_buffers)
    {
        vkDestroyBuffer(m_vulkan_device, uniform_buffer, nullptr);
    }

    for (auto uniform_buffer_memory : m_uniform_buffers_memory)
    {
        vkFreeMemory(m_vulkan_device, uniform_buffer_memory, nullptr);
    }

    for (auto framebuffer : m_swap_chain_framebuffers)
    {
        vkDestroyFramebuffer(m_vulkan_device, framebuffer, nullptr);
    }

    vkDestroyPipeline(m_vulkan_device, m_graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(m_vulkan_device, m_pipeline_layout, nullptr);
    vkDestroyRenderPass(m_vulkan_device, m_render_pass, nullptr);
}

bool Renderer::init()
{
    bool success = createRenderPass();

    if (!success)
    {
        printf("Error: Couldn't create render pass\n");
        return false;
    }

    success = createDescriptorSetLayout();

    if (!success)
    {
        printf("Error: Couldn't create descriptor set layout\n");
        return false;
    }

    success = createPipelineLayout();

    if (!success)
    {
        printf("Error: Couldn't create pipeline layout\n");
        return false;
    }

    success = createGraphicsPipeline();

    if (!success)
    {
        printf("Error: Couldn't create graphics pipeline\n");
        return false;
    }

    success = createFramebuffers();

    if (!success)
    {
        printf("Error: Couldn't create framebuffers\n");
        return false;
    }

    success = createUniformBuffers();

    if (!success)
    {
        printf("Error: Couldn't create uniform buffers\n");
        return false;
    }

    return true;
}

bool Renderer::createRenderPass()
{
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = m_vulkan_context->getSwapChainImageFormat();
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depth_attachment = {};
    depth_attachment.format = m_vulkan_context->getDepthImage()->getFormat();
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref = {};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {color_attachment,
                                                          depth_attachment};
    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = (uint32_t)(attachments.size());
    render_pass_info.pAttachments = &attachments[0];
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    VkResult result = vkCreateRenderPass(m_vulkan_device, &render_pass_info,
                                         nullptr, &m_render_pass);

    return (result == VK_SUCCESS);
}

bool Renderer::createPipelineLayout()
{
    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &m_descriptor_set_layout;

    VkResult result = vkCreatePipelineLayout(m_vulkan_device, &pipeline_layout_info,
                                             nullptr, &m_pipeline_layout);

    return (result == VK_SUCCESS);
}

bool Renderer::createGraphicsPipeline()
{
    VkShaderModule shader_module_vert;
    VkShaderModule shader_module_frag;

    bool success = createShaderModule("draw_vert.spv", &shader_module_vert);

    if (!success)
        return false;

    success = createShaderModule("draw_frag.spv", &shader_module_frag);

    if (!success)
    {
        vkDestroyShaderModule(m_vulkan_device, shader_module_vert, nullptr);
        return false;
    }

    VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
    vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = shader_module_vert;
    vert_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
    frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = shader_module_frag;
    frag_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info,
                                                       frag_shader_stage_info};

    VkVertexInputBindingDescription binding_description = {};
    binding_description.binding = 0;
    binding_description.stride = sizeof(Vertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions = {};
    attribute_descriptions[0].binding = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[0].offset = offsetof(Vertex, pos);
    attribute_descriptions[1].binding = 0;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[1].offset = offsetof(Vertex, color);
    attribute_descriptions[2].binding = 0;
    attribute_descriptions[2].location = 2;
    attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[2].offset = offsetof(Vertex, tex_coord);

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.vertexAttributeDescriptionCount = (uint32_t)(attribute_descriptions.size());
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_info.pVertexAttributeDescriptions = &attribute_descriptions[0];

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_vulkan_context->getSwapChainExtent().width;
    viewport.height = (float)m_vulkan_context->getSwapChainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = m_vulkan_context->getSwapChainExtent();

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                            VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT |
                                            VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blending = {};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pDepthStencilState = &depth_stencil;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.layout = m_pipeline_layout;
    pipeline_info.renderPass = m_render_pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    VkResult result = vkCreateGraphicsPipelines(m_vulkan_device, VK_NULL_HANDLE, 1,
                                                &pipeline_info, nullptr,
                                                &m_graphics_pipeline);

    vkDestroyShaderModule(m_vulkan_device, shader_module_frag, nullptr);
    vkDestroyShaderModule(m_vulkan_device, shader_module_vert, nullptr);

    return (result == VK_SUCCESS);
}

bool Renderer::createFramebuffers()
{
    const std::vector<VkImageView>& image_views = m_vulkan_context->getSwapChainImageViews();

    for (unsigned int i = 0; i < image_views.size(); i++)
    {
        std::array<VkImageView, 2> attachments =
        {
            image_views[i],
            m_vulkan_context->getDepthImage()->getImageView()
        };

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = m_render_pass;
        framebuffer_info.attachmentCount = (uint32_t)(attachments.size());
        framebuffer_info.pAttachments = &attachments[0];
        framebuffer_info.width = m_vulkan_context->getSwapChainExtent().width;
        framebuffer_info.height = m_vulkan_context->getSwapChainExtent().height;
        framebuffer_info.layers = 1;

        VkFramebuffer swap_chain_framebuffer = VK_NULL_HANDLE;
        VkResult result = vkCreateFramebuffer(m_vulkan_device, &framebuffer_info,
                                              nullptr, &swap_chain_framebuffer);
        if (result != VK_SUCCESS)
            return false;
        
        m_swap_chain_framebuffers.push_back(swap_chain_framebuffer);
    }

    return true;
}

bool Renderer::createUniformBuffers()
{
    unsigned int count = m_vulkan_context->getSwapChainImagesCount();
    
    for (unsigned int i = 0; i < count; i++)
    {
        VkBuffer uniform_buffer = VK_NULL_HANDLE;
        VkDeviceMemory uniform_buffer_memory = VK_NULL_HANDLE;
        
        bool success = m_vulkan_context->createBuffer(sizeof(UniformBufferObject),
                                           VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                           uniform_buffer, uniform_buffer_memory);

        if (!success)
            return false;
        
        m_uniform_buffers.push_back(uniform_buffer);
        m_uniform_buffers_memory.push_back(uniform_buffer_memory);
    }

    return true;
}

bool Renderer::createDescriptorPool(unsigned int models_count)
{
    uint32_t descriptor_count = models_count * m_vulkan_context->getSwapChainImagesCount();

    std::array<VkDescriptorPoolSize, 2> pool_sizes = {};
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = descriptor_count;
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = descriptor_count;

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = (uint32_t)(pool_sizes.size());
    pool_info.pPoolSizes = &pool_sizes[0];
    pool_info.maxSets = descriptor_count;

    VkResult result = vkCreateDescriptorPool(m_vulkan_device, &pool_info,
                                             nullptr, &m_descriptor_pool);

    return (result == VK_SUCCESS);
}

bool Renderer::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding ubo_layout_binding = {};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.pImmutableSamplers = nullptr;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding sampler_layout_binding = {};
    sampler_layout_binding.binding = 1;
    sampler_layout_binding.descriptorCount = 1;
    sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.pImmutableSamplers = nullptr;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {ubo_layout_binding,
                                                            sampler_layout_binding};

    VkDescriptorSetLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = (uint32_t)(bindings.size());
    layout_info.pBindings = &bindings[0];

    VkResult result = vkCreateDescriptorSetLayout(m_vulkan_device, &layout_info,
                                                  nullptr, &m_descriptor_set_layout);

    return (result == VK_SUCCESS);
}

bool Renderer::buildCommandBuffers(std::vector<Model*>& models)
{
    m_models = models;

    const std::vector<VkCommandBuffer> command_buffers = m_vulkan_context->getCommandBuffers();

    for (unsigned int i = 0; i < command_buffers.size(); i++)
    {
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        VkResult result = vkBeginCommandBuffer(command_buffers[i], &begin_info);

        if (result != VK_SUCCESS)
            return false;

        std::array<VkClearValue, 2> clear_values = {};
        clear_values[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clear_values[1].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = m_render_pass;
        render_pass_info.framebuffer = m_swap_chain_framebuffers[i];
        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = m_vulkan_context->getSwapChainExtent();
        render_pass_info.clearValueCount = (uint32_t)(clear_values.size());
        render_pass_info.pClearValues = &clear_values[0];

        vkCmdBeginRenderPass(command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphics_pipeline);

        for (Model* model : m_models)
        {
            VkBuffer vertex_buffers[] = {model->getVertexBuffer()};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(command_buffers[i], 0, 1, vertex_buffers, offsets);
            vkCmdBindIndexBuffer(command_buffers[i], model->getIndexBuffer(), 0,
                                 VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(command_buffers[i],
                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    m_pipeline_layout, 0, 1,
                                    &model->getDescriptorSets()[i], 0, nullptr);

            vkCmdDrawIndexed(command_buffers[i],
                            (uint32_t)(model->getIndices().size()), 1, 0, 0, 0);
        }

        vkCmdEndRenderPass(command_buffers[i]);

        result = vkEndCommandBuffer(command_buffers[i]);

        if (result != VK_SUCCESS)
            return false;
    }

    return true;
}

bool Renderer::recreateSwapChain(int drawable_width, int drawable_height)
{
    m_vulkan_context->waitIdle();

    for (auto framebuffer : m_swap_chain_framebuffers)
    {
        vkDestroyFramebuffer(m_vulkan_device, framebuffer, nullptr);
    }
    
    m_swap_chain_framebuffers.clear();

    vkDestroyPipeline(m_vulkan_device, m_graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(m_vulkan_device, m_pipeline_layout, nullptr);
    vkDestroyRenderPass(m_vulkan_device, m_render_pass, nullptr);

    bool success = m_vulkan_context->recreateSwapChain(drawable_width,
                                                       drawable_height);

    if (!success)
        return false;

    createRenderPass();
    createPipelineLayout();
    createGraphicsPipeline();
    createFramebuffers();
    buildCommandBuffers(m_models);

    return true;
}

void Renderer::updateUniformBuffer(uint32_t current_image)
{
    UniformBufferObject ubo = {};
    ubo.model = glm::mat4(1.0f);
    ubo.view = Camera::getCamera()->getViewMatrix();
    ubo.proj = Camera::getCamera()->getProjMatrix();

    void* data;
    vkMapMemory(m_vulkan_device, m_uniform_buffers_memory[current_image], 0,
                sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(m_vulkan_device, m_uniform_buffers_memory[current_image]);
}

bool Renderer::createShaderModule(std::string filename, VkShaderModule* shader_module)
{
    FileManager* file_manager = FileManager::getFileManager();

    File* file = file_manager->loadFile(filename);

    if (file == nullptr)
        return false;

    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = file->length;
    create_info.pCode = (const uint32_t*)(file->data);

    VkResult result = vkCreateShaderModule(m_vulkan_device, &create_info,
                                           nullptr, shader_module);

    file_manager->closeFile(file);

    return (result == VK_SUCCESS);
}

bool Renderer::drawFrame()
{
    bool success = m_vulkan_context->beginFrame();

    if (!success)
        return false;

    updateUniformBuffer(m_vulkan_context->getImageIndex());

    m_vulkan_context->submitCommandBuffer();

    success = m_vulkan_context->endFrame();

    if (!success)
        return false;
    
    return true;
}
