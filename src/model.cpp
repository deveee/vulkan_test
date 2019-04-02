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

#include "file_manager.hpp"
#include "renderer.hpp"
#include "texture_manager.hpp"

#include <algorithm>
#include <array>
#include <cstring>

Model::Model(std::string name,
             const std::vector<Vertex>& vertices,
             const std::vector<uint32_t>& indices,
             std::string tex_name)
{
    m_vulkan_context = VulkanContext::getVulkanContext();
    m_vulkan_device = m_vulkan_context->getDevice();

    m_name = name;
    m_vertices = vertices;
    m_indices = indices;
    m_tex_name = tex_name;

    m_vertex_buffer = VK_NULL_HANDLE;
    m_vertex_buffer_memory = VK_NULL_HANDLE;
    m_index_buffer = VK_NULL_HANDLE;
    m_index_buffer_memory = VK_NULL_HANDLE;
}

Model::~Model()
{
    if (m_index_buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(m_vulkan_device, m_index_buffer, nullptr);
    }
    
    if (m_index_buffer_memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(m_vulkan_device, m_index_buffer_memory, nullptr);
    }

    if (m_vertex_buffer != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(m_vulkan_device, m_vertex_buffer, nullptr);
    }

    if (m_vertex_buffer_memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(m_vulkan_device, m_vertex_buffer_memory, nullptr);
    }
}

bool Model::init()
{
    bool success = createVertexBuffer();

    if (!success)
    {
        printf("Error: Couldn't create vertex buffer\n");
        return false;
    }

    success = createIndexBuffer();

    if (!success)
    {
        printf("Error: Couldn't create index buffer\n");
        return false;
    }

    success = createDescriptorSets();

    if (!success)
    {
        printf("Error: Couldn't create descriptor sets\n");
        return false;
    }

    return true;
}

bool Model::createVertexBuffer()
{
    VkDeviceSize buffer_size = sizeof(m_vertices[0]) * m_vertices.size();

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    bool success = m_vulkan_context->createBuffer(buffer_size,
                                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                        staging_buffer, staging_buffer_memory);

    if (!success)
        return false;

    void* data;
    vkMapMemory(m_vulkan_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, &m_vertices[0], (size_t) buffer_size);
    vkUnmapMemory(m_vulkan_device, staging_buffer_memory);

    success = m_vulkan_context->createBuffer(buffer_size,
                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                        m_vertex_buffer, m_vertex_buffer_memory);

    if (!success)
    {
        vkDestroyBuffer(m_vulkan_device, staging_buffer, nullptr);
        vkFreeMemory(m_vulkan_device, staging_buffer_memory, nullptr);
        return false;
    }

    m_vulkan_context->copyBuffer(staging_buffer, m_vertex_buffer, buffer_size);

    vkDestroyBuffer(m_vulkan_device, staging_buffer, nullptr);
    vkFreeMemory(m_vulkan_device, staging_buffer_memory, nullptr);

    return true;
}

bool Model::createIndexBuffer()
{
    VkDeviceSize buffer_size = sizeof(m_indices[0]) * m_indices.size();

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    bool success = m_vulkan_context->createBuffer(buffer_size,
                                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                        staging_buffer, staging_buffer_memory);

    if (!success)
        return false;

    void* data;
    vkMapMemory(m_vulkan_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, &m_indices[0], (size_t) buffer_size);
    vkUnmapMemory(m_vulkan_device, staging_buffer_memory);

    success = m_vulkan_context->createBuffer(buffer_size,
                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                        m_index_buffer, m_index_buffer_memory);

    if (!success)
    {
        vkDestroyBuffer(m_vulkan_device, staging_buffer, nullptr);
        vkFreeMemory(m_vulkan_device, staging_buffer_memory, nullptr);
        return false;
    }

    m_vulkan_context->copyBuffer(staging_buffer, m_index_buffer, buffer_size);

    vkDestroyBuffer(m_vulkan_device, staging_buffer, nullptr);
    vkFreeMemory(m_vulkan_device, staging_buffer_memory, nullptr);

    return true;
}

bool Model::createDescriptorSets()
{
    Renderer* renderer = Renderer::getRenderer();

    unsigned int count = m_vulkan_context->getSwapChainImagesCount();
    std::vector<VkDescriptorSet> descriptor_sets(count);
    
    VkDescriptorSetLayout layout = renderer->getDescriptorSetLayout();
    std::vector<VkDescriptorSetLayout> layouts(count, layout);

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = renderer->getDescriptorPool();
    alloc_info.descriptorSetCount = (uint32_t)(layouts.size());
    alloc_info.pSetLayouts = &layouts[0];

    VkResult result = vkAllocateDescriptorSets(m_vulkan_device, &alloc_info, 
                                               &descriptor_sets[0]);

    if (result != VK_SUCCESS)
        return false;
    
    m_descriptor_sets = descriptor_sets;

    Texture* texture = TextureManager::getTextureManager()->getTexture(m_tex_name);
    
    if (texture == nullptr)
    {
        printf("Error: Missing texture: %s\n", m_tex_name.c_str());
        return false;
    }
        
    VulkanImage* vulkan_image = texture->vulkan_image;
    const std::vector<VkBuffer> uniform_buffers = renderer->getUniformBuffers();

    for (unsigned int i = 0; i < m_descriptor_sets.size(); i++)
    {
        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = uniform_buffers[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo image_info = {};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = vulkan_image->getImageView();
        image_info.sampler = vulkan_image->getSampler();

        std::array<VkWriteDescriptorSet, 2> write_descriptor_sets = {};
        write_descriptor_sets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_sets[0].dstSet = m_descriptor_sets[i];
        write_descriptor_sets[0].dstBinding = 0;
        write_descriptor_sets[0].dstArrayElement = 0;
        write_descriptor_sets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_descriptor_sets[0].descriptorCount = 1;
        write_descriptor_sets[0].pBufferInfo = &buffer_info;
        write_descriptor_sets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_sets[1].dstSet = m_descriptor_sets[i];
        write_descriptor_sets[1].dstBinding = 1;
        write_descriptor_sets[1].dstArrayElement = 0;
        write_descriptor_sets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_descriptor_sets[1].descriptorCount = 1;
        write_descriptor_sets[1].pImageInfo = &image_info;

        vkUpdateDescriptorSets(m_vulkan_device, (uint32_t)(write_descriptor_sets.size()),
                               &write_descriptor_sets[0], 0, nullptr);
    }

    return true;
}

