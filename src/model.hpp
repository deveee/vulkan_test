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

#ifndef MODEL_HPP
#define MODEL_HPP

#include "vulkan_context.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <map>
#include <string>

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 tex_coord;
};

class Model
{
private:
    VulkanContext* m_vulkan_context;
    VkDevice m_vulkan_device;

    std::string m_name;
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    std::string m_tex_name;

    VkBuffer m_vertex_buffer;
    VkDeviceMemory m_vertex_buffer_memory;
    VkBuffer m_index_buffer;
    VkDeviceMemory m_index_buffer_memory;
    std::vector<VkDescriptorSet> m_descriptor_sets;

    bool createVertexBuffer();
    bool createIndexBuffer();
    bool createDescriptorSets();

public:
    Model(std::string name,
          const std::vector<Vertex>& vertices,
          const std::vector<uint32_t>& indices,
          std::string tex_name);
    ~Model();

    bool init();

    const std::vector<Vertex>& getVertices() {return m_vertices;}
    const std::vector<uint32_t>& getIndices() {return m_indices;}
    std::string getName() {return m_name;}
    std::string getTexName() {return m_tex_name;}

    const VkBuffer getVertexBuffer() {return m_vertex_buffer;}
    const VkDeviceMemory getVertexBufferMemory() {return m_vertex_buffer_memory;}
    const VkBuffer getIndexBuffer() {return m_index_buffer;}
    const VkDeviceMemory getIndexBufferMemory() {return m_index_buffer_memory;}
    const std::vector<VkDescriptorSet>& getDescriptorSets() {return m_descriptor_sets;}
};

#endif
