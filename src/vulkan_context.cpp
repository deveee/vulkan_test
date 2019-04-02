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

#include <algorithm>
#include <set>
#include <string>

const unsigned int MAX_FRAMES_IN_FLIGHT = 2;

VulkanContext* VulkanContext::m_vulkan_context = nullptr;

#if defined(__linux__) && !defined(ANDROID)
VulkanContext::VulkanContext(Display* display, Window window, 
                             uint32_t drawable_width, uint32_t drawable_height)
#elif defined(ANDROID)
VulkanContext::VulkanContext(ANativeWindow* window, 
                             uint32_t drawable_width, uint32_t drawable_height)
#else
    #error Unsupported system
#endif
{
    m_vulkan_context = this;

    m_instance = VK_NULL_HANDLE;
    m_surface = VK_NULL_HANDLE;
    m_physical_device = VK_NULL_HANDLE;
    m_device = VK_NULL_HANDLE;
    m_graphics_queue = VK_NULL_HANDLE;
    m_present_queue = VK_NULL_HANDLE;
    m_swap_chain = VK_NULL_HANDLE;
    m_command_pool = VK_NULL_HANDLE;
    m_depth_image = nullptr;

    m_current_frame = 0;
    m_image_index = 0;
    m_swap_chain_images_count = 0;

    m_graphics_family = 0;
    m_present_family = 0;
    m_drawable_width = drawable_width;
    m_drawable_height = drawable_height;

#if defined(__linux__) && !defined(ANDROID)
    m_display = display;
    m_window = window;
#elif defined(ANDROID)
    m_window = window;
#else
    #error Unsupported system
#endif

    m_device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

VulkanContext::~VulkanContext()
{
    delete m_depth_image;

    if (!m_command_buffers.empty())
    {
        vkFreeCommandBuffers(m_device, m_command_pool, 
                            (uint32_t)(m_command_buffers.size()), 
                            &m_command_buffers[0]);
    }
    
    if (m_command_pool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(m_device, m_command_pool, nullptr);
    }

    for (VkSemaphore& semaphore : m_render_finished_semaphores)
    {
        vkDestroySemaphore(m_device, semaphore, nullptr);
    }

    for (VkSemaphore& semaphore : m_image_available_semaphores)
    {
        vkDestroySemaphore(m_device, semaphore, nullptr);
    }

    for (VkFence& fence : m_in_flight_fences)
    {
        vkDestroyFence(m_device, fence, nullptr);
    }

    for (VkImageView& image_view : m_swap_chain_image_views)
    {
        vkDestroyImageView(m_device, image_view, nullptr);
    }

    if (m_swap_chain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(m_device, m_swap_chain, nullptr);
    }
    
    if (m_device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(m_device, nullptr);
    }
    
    if (m_surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    }
    
    if (m_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_instance, nullptr);
    }
}

bool VulkanContext::init()
{
    bool success = createInstance();

    if (!success)
    {
        printf("Error: Couldn't create instance\n");
        return false;
    }

    success = createSurface();

    if (!success)
    {
        printf("Error: Couldn't create surface\n");
        return false;
    }

    success = findPhysicalDevice();

    if (!success)
    {
        printf("Error: Couldn't find physical device\n");
        return false;
    }

    success = createDevice();

    if (!success)
    {
        printf("Error: Couldn't create vulkan device\n");
        return false;
    }

    success = createSwapChain();

    if (!success)
    {
        printf("Error: Couldn't create swap chain\n");
        return false;
    }

    success = createSyncObjects();

    if (!success)
    {
        printf("Error: Couldn't create sync objects\n");
        return false;
    }

    success = createCommandPool();

    if (!success)
    {
        printf("Error: Couldn't create command pool\n");
        return false;
    }

    success = createCommandBuffers();

    if (!success)
    {
        printf("Error: Couldn't create command buffers\n");
        return false;
    }

    success = createDepthBuffer();

    if (!success)
    {
        printf("Error: Couldn't create depth buffer\n");
        return false;
    }

    return true;
}

bool VulkanContext::createInstance()
{
    VkApplicationInfo application_info = {};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "Application Name";
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pEngineName = "No Engine";
    application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.apiVersion = VK_API_VERSION_1_0;

#if defined(__linux__) && !defined(ANDROID)
    std::vector<const char*> extensions = {VK_KHR_SURFACE_EXTENSION_NAME,
                                           VK_KHR_XLIB_SURFACE_EXTENSION_NAME};
#elif defined(ANDROID)
    std::vector<const char*> extensions = {VK_KHR_SURFACE_EXTENSION_NAME,
                                           VK_KHR_ANDROID_SURFACE_EXTENSION_NAME};
#else
    #error Unsupported system
#endif

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &application_info;
    create_info.enabledExtensionCount = (uint32_t)(extensions.size());
    create_info.ppEnabledExtensionNames = &extensions[0];
    create_info.enabledLayerCount = 0;

    VkResult result = vkCreateInstance(&create_info, nullptr, &m_instance);

    return (result == VK_SUCCESS);
}

bool VulkanContext::createSurface()
{
#if defined(__linux__) && !defined(ANDROID)
    VkXlibSurfaceCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    create_info.dpy = m_display;
    create_info.window = m_window;

    VkResult result = vkCreateXlibSurfaceKHR(m_instance, &create_info,
                                             nullptr, &m_surface);

    return (result == VK_SUCCESS);
#elif defined(ANDROID)
    VkAndroidSurfaceCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    create_info.window = m_window;
    
    VkResult result = vkCreateAndroidSurfaceKHR(m_instance, &create_info, 
                                                nullptr, &m_surface);
    
    return (result == VK_SUCCESS);
#else
    #error Unsupported system
#endif
}

bool VulkanContext::findPhysicalDevice()
{
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);

    if (device_count < 1)
        return false;

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(m_instance, &device_count, &devices[0]);

    for (VkPhysicalDevice& device : devices)
    {
        uint32_t graphics_family = 0;
        uint32_t present_family = 0;

        bool success = findQueueFamilies(device, &graphics_family, &present_family);

        if (!success)
            continue;

        success = checkDeviceExtensions(device);

        if (!success)
            continue;

        VkSurfaceCapabilitiesKHR surface_capabilities;
        std::vector<VkSurfaceFormatKHR> surface_formats;
        std::vector<VkPresentModeKHR> present_modes;

        success = updateSurfaceInformation(device, &surface_capabilities, 
                                           &surface_formats, &present_modes);

        if (!success)
            continue;

        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceFeatures(device, &device_features);

        if (!device_features.samplerAnisotropy)
            continue;

        m_graphics_family = graphics_family;
        m_present_family = present_family;
        m_surface_capabilities = surface_capabilities;
        m_surface_formats = surface_formats;
        m_present_modes = present_modes;
        m_physical_device = device;
        break;
    }

    return (m_physical_device != VK_NULL_HANDLE);
}

bool VulkanContext::createDevice()
{
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = m_graphics_family;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos.push_back(queue_create_info);

    queue_create_info.queueFamilyIndex = m_present_family;
    queue_create_infos.push_back(queue_create_info);

    VkPhysicalDeviceFeatures device_features = {};
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = (uint32_t)(queue_create_infos.size());
    create_info.pQueueCreateInfos = &queue_create_infos[0];
    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount = (uint32_t)(m_device_extensions.size());
    create_info.ppEnabledExtensionNames = &m_device_extensions[0];
    create_info.enabledLayerCount = 0;

    VkResult result = vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device);

    if (result != VK_SUCCESS)
        return false;

    vkGetDeviceQueue(m_device, m_graphics_family, 0, &m_graphics_queue);
    vkGetDeviceQueue(m_device, m_present_family, 0, &m_present_queue);

    return true;
}

bool VulkanContext::createSwapChain()
{
    VkSurfaceFormatKHR surface_format = m_surface_formats[0];

    if (m_surface_formats.size() == 1 && 
        m_surface_formats[0].format == VK_FORMAT_UNDEFINED)
    {
        surface_format = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }
    else
    {
        for (VkSurfaceFormatKHR& available_format : m_surface_formats)
        {
            if (available_format.format == VK_FORMAT_B8G8R8A8_UNORM &&
                available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                surface_format = available_format;
                break;
            }
        }
    }

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

    for (VkPresentModeKHR& available_mode : m_present_modes)
    {
        if (available_mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            present_mode = available_mode;
            break;
        }
        else if (available_mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            present_mode = available_mode;
        }
    }

    VkExtent2D image_extent = m_surface_capabilities.currentExtent;

    if (m_surface_capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
    {
        VkExtent2D max_extent = m_surface_capabilities.maxImageExtent;
        VkExtent2D min_extent = m_surface_capabilities.minImageExtent;

        VkExtent2D actual_extent = {std::max(std::min((uint32_t)m_drawable_width,
                                    max_extent.width), min_extent.width),
                                    std::max(std::min((uint32_t)m_drawable_height,
                                    max_extent.height), min_extent.height)};

        image_extent = actual_extent;
    }

    m_swap_chain_images_count = m_surface_capabilities.minImageCount + 1;

    if (m_surface_capabilities.maxImageCount > 0)
    {
        m_swap_chain_images_count = std::min(m_swap_chain_images_count, 
                                             m_surface_capabilities.maxImageCount);
    }

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = m_surface;
    create_info.minImageCount = m_swap_chain_images_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = image_extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (m_graphics_family != m_present_family)
    {
        uint32_t queueFamilyIndices[] = {m_graphics_family, m_present_family};
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform = m_surface_capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(m_device, &create_info, nullptr, 
                                           &m_swap_chain);

    if (result != VK_SUCCESS)
        return false;

    vkGetSwapchainImagesKHR(m_device, m_swap_chain, &m_swap_chain_images_count, nullptr);

    m_swap_chain_images.resize(m_swap_chain_images_count);
    vkGetSwapchainImagesKHR(m_device, m_swap_chain, &m_swap_chain_images_count, 
                            &m_swap_chain_images[0]);

    m_swap_chain_image_format = surface_format.format;
    m_swap_chain_extent = image_extent;

    for (unsigned int i = 0; i < m_swap_chain_images.size(); i++)
    {
        VkImageViewCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = m_swap_chain_images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = m_swap_chain_image_format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        VkImageView swap_chain_image_view = VK_NULL_HANDLE;
        VkResult result = vkCreateImageView(m_device, &create_info, nullptr, 
                                            &swap_chain_image_view);

        if (result != VK_SUCCESS)
            return false;
        
        m_swap_chain_image_views.push_back(swap_chain_image_view);
    }

    return true;
}

bool VulkanContext::createSyncObjects()
{
    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (unsigned int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkSemaphore image_available_semaphore;
        VkResult result = vkCreateSemaphore(m_device, &semaphore_info, nullptr, 
                                            &image_available_semaphore);

        if (result != VK_SUCCESS)
            return false;

        VkSemaphore render_finished_semaphore;
        result = vkCreateSemaphore(m_device, &semaphore_info, nullptr, 
                                   &render_finished_semaphore);

        if (result != VK_SUCCESS)
            return false;

        VkFence in_flight_fence;
        result = vkCreateFence(m_device, &fence_info, nullptr, &in_flight_fence);

        if (result != VK_SUCCESS)
            return false;

        m_image_available_semaphores.push_back(image_available_semaphore);
        m_render_finished_semaphores.push_back(render_finished_semaphore);
        m_in_flight_fences.push_back(in_flight_fence);
    }

    return true;
}

bool VulkanContext::createCommandPool()
{
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = m_graphics_family;

    VkResult result = vkCreateCommandPool(m_device, &pool_info, nullptr, 
                                          &m_command_pool);

    return (result == VK_SUCCESS);
}

bool VulkanContext::createCommandBuffers()
{
    std::vector<VkCommandBuffer> command_buffers(m_swap_chain_images.size());

    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = m_command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = (uint32_t)command_buffers.size();

    VkResult result = vkAllocateCommandBuffers(m_device, &alloc_info, 
                                               &command_buffers[0]);

    if (result != VK_SUCCESS)
        return false;
    
    m_command_buffers = command_buffers;
    
    return true;
}

bool VulkanContext::createDepthBuffer()
{
    VkFormat depth_format = VK_FORMAT_UNDEFINED;

    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    std::vector<VkFormat> formats = {VK_FORMAT_D32_SFLOAT,
                                     VK_FORMAT_D32_SFLOAT_S8_UINT,
                                     VK_FORMAT_D24_UNORM_S8_UINT};

    for (VkFormat format : formats)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_physical_device, format, &props);

        if ((props.optimalTilingFeatures & features) == features)
        {
            depth_format = format;
            break;
        }
    }

    if (depth_format == VK_FORMAT_UNDEFINED)
        return false;

    m_depth_image = new VulkanImage(depth_format, m_swap_chain_extent.width,
                                    m_swap_chain_extent.height);

    bool success = m_depth_image->createImage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    if (!success)
        return false;

    success = m_depth_image->createImageView(VK_IMAGE_ASPECT_DEPTH_BIT);

    if (!success)
        return false;

    m_depth_image->transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    return true;
}

bool VulkanContext::checkDeviceExtensions(VkPhysicalDevice device)
{
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, 
                                         &extensions[0]);

    std::set<std::string> required_extensions(m_device_extensions.begin(), 
                                              m_device_extensions.end());

    for (VkExtensionProperties& extension : extensions)
    {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

bool VulkanContext::updateSurfaceInformation(VkPhysicalDevice device,
                                             VkSurfaceCapabilitiesKHR* surface_capabilities,
                                             std::vector<VkSurfaceFormatKHR>* surface_formats,
                                             std::vector<VkPresentModeKHR>* present_modes)
{
    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, nullptr);

    if (format_count < 1)
        return false;

    uint32_t mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &mode_count, nullptr);

    if (mode_count < 1)
        return false;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, surface_capabilities);

    (*surface_formats).resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count, 
                                         &(*surface_formats)[0]);

    (*present_modes).resize(mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &mode_count, 
                                              &(*present_modes)[0]);

    return true;
}

bool VulkanContext::findQueueFamilies(VkPhysicalDevice device, uint32_t* graphics_family, 
                                      uint32_t* present_family)
{
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, 
                                             &queue_families[0]);

    bool found_graphics_family = false;
    bool found_present_family = false;

    for (unsigned int i = 0; i < queue_families.size(); i++)
    {
        if (queue_families[i].queueCount > 0 &&
            queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            *graphics_family = i;
            found_graphics_family = true;
            break;
        }
    }

    for (unsigned int i = 0; i < queue_families.size(); i++)
    {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

        if (queue_families[i].queueCount > 0 && presentSupport)
        {
            *present_family = i;
            found_present_family = true;
            break;
        }
    }

    return found_graphics_family && found_present_family;
}

bool VulkanContext::recreateSwapChain(uint32_t drawable_width, uint32_t drawable_height)
{
    delete m_depth_image;

    vkFreeCommandBuffers(m_device, m_command_pool, (uint32_t)(m_command_buffers.size()), 
                         &m_command_buffers[0]);
                         
    m_command_buffers.clear();

    for (VkImageView& image_view : m_swap_chain_image_views)
    {
        vkDestroyImageView(m_device, image_view, nullptr);
    }
    
    m_swap_chain_image_views.clear();

    vkDestroySwapchainKHR(m_device, m_swap_chain, nullptr);

    m_drawable_width = drawable_width;
    m_drawable_height = drawable_height;

    bool success = updateSurfaceInformation(m_physical_device, &m_surface_capabilities,
                                            &m_surface_formats, &m_present_modes);

    if (!success)
        return false;

    success = createSwapChain();

    if (!success)
        return false;

    success = createCommandBuffers();

    if (!success)
        return false;

    success = createDepthBuffer();

    if (!success)
        return false;

    return true;
}

void VulkanContext::waitIdle()
{
    vkDeviceWaitIdle(m_device);
}

bool VulkanContext::beginFrame()
{
    VkFence fence = m_in_flight_fences[m_current_frame];
    vkWaitForFences(m_device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(m_device, 1, &fence);

    VkSemaphore semaphore = m_image_available_semaphores[m_current_frame];
    VkResult result = vkAcquireNextImageKHR(m_device, m_swap_chain, 
                                            std::numeric_limits<uint64_t>::max(),
                                            semaphore, VK_NULL_HANDLE, &m_image_index);

    return (result != VK_ERROR_OUT_OF_DATE_KHR);
}

bool VulkanContext::endFrame()
{
    VkSemaphore semaphores[] = {m_render_finished_semaphores[m_current_frame]};
    VkSwapchainKHR swap_chains[] = {m_swap_chain};

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = semaphores;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &m_image_index;

    m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

    VkResult result = vkQueuePresentKHR(m_present_queue, &present_info);

    return (result != VK_ERROR_OUT_OF_DATE_KHR && result != VK_SUBOPTIMAL_KHR);
}

bool VulkanContext::submitCommandBuffer()
{
    VkSemaphore wait_semaphores[] = {m_image_available_semaphores[m_current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signal_semaphores[] = {m_render_finished_semaphores[m_current_frame]};

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_command_buffers[m_image_index];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    VkResult result = vkQueueSubmit(m_graphics_queue, 1, &submit_info, 
                                    m_in_flight_fences[m_current_frame]);

    return (result == VK_SUCCESS);
}

VkCommandBuffer VulkanContext::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = m_command_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(m_device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);

    return command_buffer;
}

void VulkanContext::endSingleTimeCommands(VkCommandBuffer command_buffer)
{
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(m_graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphics_queue);

    vkFreeCommandBuffers(m_device, m_command_pool, 1, &command_buffer);
}

bool VulkanContext::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, 
                                 VkMemoryPropertyFlags properties, VkBuffer& buffer, 
                                 VkDeviceMemory& buffer_memory)
{
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(m_device, &buffer_info, nullptr, &buffer);

    if (result != VK_SUCCESS)
        return false;

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(m_device, buffer, &mem_requirements);

    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(m_physical_device, &mem_properties);

    uint32_t memory_type_index = std::numeric_limits<uint32_t>::max();
    uint32_t type_filter = mem_requirements.memoryTypeBits;

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

    result = vkAllocateMemory(m_device, &alloc_info, nullptr, &buffer_memory);

    if (result != VK_SUCCESS)
        return false;

    vkBindBufferMemory(m_device, buffer, buffer_memory, 0);

    return true;
}

void VulkanContext::copyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, 
                               VkDeviceSize size)
{
    VkCommandBuffer command_buffer = beginSingleTimeCommands();

    VkBufferCopy copy_region = {};
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

    endSingleTimeCommands(command_buffer);
}
