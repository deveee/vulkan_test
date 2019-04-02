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

#ifndef VULKAN_IMAGE_HPP
#define VULKAN_IMAGE_HPP

#include <vulkan/vulkan.h>

class VulkanContext;

class VulkanImage
{
private:
    VulkanContext* m_vulkan_context;
    VkDevice m_vulkan_device;

    VkImage m_image;
    VkDeviceMemory m_image_memory;
    VkImageView m_image_view;
    VkSampler m_sampler;
    VkFormat m_format;
    unsigned int m_width;
    unsigned int m_height;

public:
    VulkanImage(VkFormat format, unsigned int width, unsigned int height);
    ~VulkanImage();

    bool createImage(VkImageUsageFlags usage);
    bool createImageView(VkImageAspectFlags aspect_flags);
    bool createTextureImage(const void* texture_data, unsigned int channels);
    bool createSampler();
    void transitionImageLayout(VkImageLayout old_layout, VkImageLayout new_layout);
    void copyBufferToImage(VkBuffer buffer);

    VkImage getImage() {return m_image;}
    VkImageView getImageView() {return m_image_view;}
    VkSampler getSampler() {return m_sampler;}
    VkFormat getFormat() {return m_format;}
};

#endif
