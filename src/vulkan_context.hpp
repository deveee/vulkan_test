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

#ifndef VULKAN_CONTEXT_HPP
#define VULKAN_CONTEXT_HPP

#if defined(__linux__) && !defined(ANDROID)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(ANDROID)
#define VK_USE_PLATFORM_ANDROID_KHR
#endif

#include <vulkan/vulkan.h>

#include "vulkan_image.hpp"

#include <vector>

class VulkanContext
{
private:
    VkInstance m_instance;
    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physical_device;
    VkDevice m_device;
    std::vector<const char*> m_device_extensions;
    VkSurfaceCapabilitiesKHR m_surface_capabilities;
    std::vector<VkSurfaceFormatKHR> m_surface_formats;
    std::vector<VkPresentModeKHR> m_present_modes;
    VkQueue m_graphics_queue;
    VkQueue m_present_queue;

    VkSwapchainKHR m_swap_chain;
    std::vector<VkImage> m_swap_chain_images;
    VkFormat m_swap_chain_image_format;
    VkExtent2D m_swap_chain_extent;
    std::vector<VkImageView> m_swap_chain_image_views;

    VulkanImage* m_depth_image;

    VkCommandPool m_command_pool;
    std::vector<VkCommandBuffer> m_command_buffers;

    std::vector<VkSemaphore> m_image_available_semaphores;
    std::vector<VkSemaphore> m_render_finished_semaphores;
    std::vector<VkFence> m_in_flight_fences;
    unsigned int m_current_frame;
    unsigned int m_swap_chain_images_count;
    uint32_t m_image_index;

    uint32_t m_graphics_family;
    uint32_t m_present_family;
    uint32_t m_drawable_width;
    uint32_t m_drawable_height;

#if defined(__linux__) && !defined(ANDROID)
    Display* m_display;
    Window m_window;
#elif defined(ANDROID)
    ANativeWindow* m_window;
#else
    #error Unsupported system
#endif

    static VulkanContext* m_vulkan_context;

    bool createInstance();
    bool createSurface();
    bool findPhysicalDevice();
    bool createDevice();
    bool createSwapChain();
    bool createSyncObjects();
    bool createCommandPool();
    bool createCommandBuffers();
    bool createDepthBuffer();
    bool checkDeviceExtensions(VkPhysicalDevice device);
    bool findQueueFamilies(VkPhysicalDevice device, uint32_t* graphics_family, uint32_t* present_family);
    bool updateSurfaceInformation(VkPhysicalDevice device,
                  VkSurfaceCapabilitiesKHR* surface_capabilities,
                  std::vector<VkSurfaceFormatKHR>* surface_formats,
                  std::vector<VkPresentModeKHR>* present_modes);

public:
#if defined(__linux__) && !defined(ANDROID)
    VulkanContext(Display* display, Window window, uint32_t drawable_width, uint32_t drawable_height);
#elif defined(ANDROID)
    VulkanContext(ANativeWindow* window, uint32_t drawable_width, uint32_t drawable_height);
#else
    #error Unsupported system
#endif

    ~VulkanContext();

    bool init();
    bool recreateSwapChain(uint32_t drawable_width, uint32_t drawable_height);
    void waitIdle();
    bool beginFrame();
    bool endFrame();
    bool submitCommandBuffer();
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer command_buffer);
    bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& buffer_memory);
    void copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);

    VkDevice getDevice() {return m_device;}
    VkPhysicalDevice getPhysicalDevice() {return m_physical_device;}
    VkFormat getSwapChainImageFormat() {return m_swap_chain_image_format;}
    VkExtent2D getSwapChainExtent() {return m_swap_chain_extent;}
    const std::vector<VkImage>& getSwapChainImages() {return m_swap_chain_images;}
    const std::vector<VkImageView>& getSwapChainImageViews() {return m_swap_chain_image_views;}
    unsigned int getSwapChainImagesCount() {return m_swap_chain_images_count;}
    const std::vector<VkCommandBuffer>& getCommandBuffers() {return m_command_buffers;}
    VkQueue getGraphicsQueue() {return m_graphics_queue;}
    uint32_t getGraphicsFamily() {return m_graphics_family;}
    uint32_t getDrawableWidth() {return m_drawable_width;}
    uint32_t getDrawableHeight() {return m_drawable_height;}
    uint32_t getImageIndex() {return m_image_index;}
    VulkanImage* getDepthImage() {return m_depth_image;}

    static VulkanContext* getVulkanContext() {return m_vulkan_context;}
};

#endif
