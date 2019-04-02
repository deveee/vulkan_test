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

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "model_manager.hpp"
#include "vulkan_context.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class Renderer
{
private:
    VulkanContext* m_vulkan_context;
    VkDevice m_vulkan_device;

    VkRenderPass m_render_pass;
    VkPipelineLayout m_pipeline_layout;
    VkPipeline m_graphics_pipeline;
    std::vector<VkFramebuffer> m_swap_chain_framebuffers;
    std::vector<VkBuffer> m_uniform_buffers;
    std::vector<VkDeviceMemory> m_uniform_buffers_memory;
    VkDescriptorPool m_descriptor_pool;
    VkDescriptorSetLayout m_descriptor_set_layout;

    std::vector<Model*> m_models;

    static Renderer* m_renderer;

    bool createRenderPass();
    bool createPipelineLayout();
    bool createGraphicsPipeline();
    bool createFramebuffers();
    bool createUniformBuffers();
    bool createDescriptorSetLayout();

    bool createShaderModule(std::string filename, VkShaderModule* shader_module);
    void updateUniformBuffer(uint32_t current_image);

public:
    Renderer();
    ~Renderer();

    bool init();
    bool buildCommandBuffers(std::vector<Model*>& models);
    bool createDescriptorPool(unsigned int models_count);
    bool recreateSwapChain(int drawable_width, int drawable_height);
    bool drawFrame();

    VkDescriptorPool getDescriptorPool() {return m_descriptor_pool;}
    VkDescriptorSetLayout getDescriptorSetLayout() {return m_descriptor_set_layout;}
    const std::vector<VkBuffer>& getUniformBuffers() {return m_uniform_buffers;}

    static Renderer* getRenderer() {return m_renderer;}
};

#endif
