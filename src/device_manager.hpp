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

#ifndef DEVICE_MANAGER_HPP
#define DEVICE_MANAGER_HPP

#include "device.hpp"
#include "vulkan_context.hpp"

class DeviceManager
{
private:
    static Device* m_device;
    VulkanContext* m_vulkan_context;
    static DeviceManager* m_device_manager;
    
    bool initWindow();
    bool initVulkanContext();
    
public:
    DeviceManager();
    ~DeviceManager();
    
    bool init();
    Device* getDevice() {return m_device;}
    VulkanContext* getVulkanContext() { return m_vulkan_context;}
    void printDeviceInfo();
    
    static DeviceManager* getDeviceManager() {return m_device_manager;}
};

#endif
