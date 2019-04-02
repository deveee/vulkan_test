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

#include "device_android.hpp"
#include "device_linux.hpp"
#include "device_manager.hpp"

#ifdef ANDROID
extern struct android_app* g_android_app;
#endif

Device* DeviceManager::m_device = nullptr;
DeviceManager* DeviceManager::m_device_manager = nullptr;

DeviceManager::DeviceManager()
{
    m_device_manager = this;
    m_device = nullptr;
    m_vulkan_context = nullptr;
}

DeviceManager::~DeviceManager()
{
    delete m_vulkan_context;
    delete m_device;
}

bool DeviceManager::init()
{
    bool success = initWindow();

    if (!success)
    {
        printf("Error: Couldn't initialize window\n");
        return false;
    }

    success = initVulkanContext();

    if (!success)
    {
        printf("Error: Couldn't initialize vulkan context\n");
        return false;
    }

    printDeviceInfo();

    return true;
}

bool DeviceManager::initWindow()
{
    CreationParams params;
    params.window_width = 1280;
    params.window_height = 720;
    params.fullscreen = false;
    params.vsync = true;
    params.handle_srgb = false;
    params.alpha_channel = false;
    params.force_legacy_device = false;
#if (defined(__linux__) || defined(__CYGWIN__)) && !defined(ANDROID)
    params.private_data = nullptr;
#elif defined(ANDROID)
    params.private_data = g_android_app;
#else
    #error Unsupported architecture
#endif
    params.joystick_support = false;

#if (defined(__linux__) || defined(__CYGWIN__)) && !defined(ANDROID)
    m_device = new DeviceLinux();
#elif defined(ANDROID)
    m_device = new DeviceAndroid();
#else
    #error Unsupported architecture
#endif

    bool success = m_device->initDevice(params);

    if (!success)
    {
        printf("Error: Couldn't initialize device.\n");
        return false;
    }

    m_device->setWindowCaption("Test vulkan");
    m_device->setWindowClass("TestVulkan");

    return true;
}

bool DeviceManager::initVulkanContext()
{
#if (defined(__linux__) || defined(__CYGWIN__)) && !defined(ANDROID)
    DeviceLinux* device_linux = (DeviceLinux*)m_device;

    m_vulkan_context = new VulkanContext(device_linux->getDisplay(),
                                         device_linux->getWindow(),
                                         device_linux->getWindowWidth(),
                                         device_linux->getWindowHeight());
#elif defined(ANDROID)
    DeviceAndroid* device_android = (DeviceAndroid*)m_device;
    
    m_vulkan_context = new VulkanContext(g_android_app->window,
                                         device_android->getWindowWidth(),
                                         device_android->getWindowHeight());
#else
    #error Unsupported architecture
#endif

    bool success = m_vulkan_context->init();

    return success;
}

void DeviceManager::printDeviceInfo()
{

}
