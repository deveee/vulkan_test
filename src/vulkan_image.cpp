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

#include "vulkan_context.hpp"
#include "vulkan_image.hpp"

#include <cassert>
#include <algorithm>
#include <cstring>

VulkanImage::VulkanImage(VkFormat format, unsigned int width, unsigned int height)
{
    m_vulkan_context = VulkanContext::getVulkanContext();
    m_vulkan_device = m_vulkan_context->getDevice();

    m_image = VK_NULL_HANDLE;
    m_image_memory = VK_NULL_HANDLE;
    m_image_view = VK_NULL_HANDLE;
    m_sampler = VK_NULL_HANDLE;
    m_format = format;
    m_width = width;
    m_height = height;
}

VulkanImage::~VulkanImage()
{
    if (m_sampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(m_vulkan_device, m_sampler, nullptr);
    }

    if (m_image_view != VK_NULL_HANDLE)
    {
        vkDestroyImageView(m_vulkan_device, m_image_view, nullptr);
    }

    if (m_image != VK_NULL_HANDLE)
    {
        vkDestroyImage(m_vulkan_device, m_image, nullptr);
    }

    if (m_image_memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(m_vulkan_device, m_image_memory, nullptr);
    }
}

bool VulkanImage::createImage(VkImageUsageFlags usage)
{
    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = m_width;
    image_info.extent.height = m_height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = m_format;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateImage(m_vulkan_device, &image_info, nullptr, &m_image);

    if (result != VK_SUCCESS)
        return false;

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(m_vulkan_device, m_image, &mem_requirements);

    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(m_vulkan_context->getPhysicalDevice(), 
                                        &mem_properties);

    uint32_t memory_type_index = std::numeric_limits<uint32_t>::max();
    uint32_t type_filter = mem_requirements.memoryTypeBits;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
    {
        if ((type_filter & (1 << i)) &&
            (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            memory_type_index = i;
            break;
        }
    }

    if (memory_type_index == std::numeric_limits<uint32_t>::max())
        return false;

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = memory_type_index;

    result = vkAllocateMemory(m_vulkan_device, &alloc_info, nullptr, &m_image_memory);

    if (result != VK_SUCCESS)
        return false;

    vkBindImageMemory(m_vulkan_device, m_image, m_image_memory, 0);

    return true;
}

bool VulkanImage::createTextureImage(const void* texture_data, unsigned int channels)
{
    assert(channels == 4);
    
    VkDeviceSize image_size = m_width * m_height * channels;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    bool success = m_vulkan_context->createBuffer(image_size,
                                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                        staging_buffer, staging_buffer_memory);

    if (!success)
        return false;

    void* data;
    vkMapMemory(m_vulkan_device, staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, texture_data, (size_t)(image_size));
    vkUnmapMemory(m_vulkan_device, staging_buffer_memory);

    success = createImage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    if (!success)
        return false;

    transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copyBufferToImage(staging_buffer);

    transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(m_vulkan_device, staging_buffer, nullptr);
    vkFreeMemory(m_vulkan_device, staging_buffer_memory, nullptr);

    return true;
}

bool VulkanImage::createImageView(VkImageAspectFlags aspect_flags)
{
    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = m_image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = m_format;
    view_info.subresourceRange.aspectMask = aspect_flags;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VkResult result = vkCreateImageView(m_vulkan_device, &view_info, nullptr, 
                                        &m_image_view);

    return (result == VK_SUCCESS);
}

bool VulkanImage::createSampler()
{
    VkSamplerCreateInfo sampler_info = {};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = 16;
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VkResult result = vkCreateSampler(m_vulkan_device, &sampler_info, nullptr, 
                                      &m_sampler);

    return (result == VK_SUCCESS);
}

void VulkanImage::transitionImageLayout(VkImageLayout old_layout,
                    VkImageLayout new_layout)
{
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    
        if (m_format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
            m_format == VK_FORMAT_D24_UNORM_S8_UINT)
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
        new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
             new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        return;
    }

    VkCommandBuffer command_buffer = m_vulkan_context->beginSingleTimeCommands();

    vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);

    m_vulkan_context->endSingleTimeCommands(command_buffer);
}

void VulkanImage::copyBufferToImage(VkBuffer buffer)
{
    VkCommandBuffer command_buffer = m_vulkan_context->beginSingleTimeCommands();

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {m_width, m_height, 1};

    vkCmdCopyBufferToImage(command_buffer, buffer, m_image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    m_vulkan_context->endSingleTimeCommands(command_buffer);
}
