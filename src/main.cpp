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
#include "device_manager.hpp"
#include "file_manager.hpp"
#include "image_loader.hpp"
#include "model_manager.hpp"
#include "renderer.hpp"
#include "texture_manager.hpp"
#include "vulkan_context.hpp"

#include <cstdio>
#include <memory>

#ifdef ANDROID
#include "device_android.hpp"

struct android_app* g_android_app;
#endif

static int mouse_pos_x = 0;
static int mouse_pos_y = 0;

static void onEvent(Event event)
{
    Camera* camera = Camera::getCamera();

    if (event.type == ET_MOUSE_EVENT)
    {
        MouseEvent mouse_event = event.mouse;

        if (mouse_event.type == ME_LEFT_PRESSED)
        {
            mouse_pos_x = mouse_event.x;
            mouse_pos_y = mouse_event.y;
        }
        else if (mouse_event.type == ME_MOUSE_MOVED &&
                 mouse_event.button_state_left)
        {
            camera->rotate((mouse_event.x - mouse_pos_x) * 0.005f, 
                           (mouse_event.y - mouse_pos_y) * 0.005f);
            
            mouse_pos_x = mouse_event.x;
            mouse_pos_y = mouse_event.y;
        }
    }
    else if (event.type == ET_KEY_EVENT)
    {
        KeyEvent key_event = event.key;

        if (key_event.pressed)
        {
            switch (key_event.id)
            {
            case KC_KEY_A:
                camera->rotate(0.05f, 0);
                break;
            case KC_KEY_D:
                camera->rotate(-0.05f, 0);
                break;
            case KC_KEY_W:
                camera->rotate(0, -0.05f);
                break;
            case KC_KEY_S:
                camera->rotate(0, 0.05f);
                break;
            case KC_KEY_ESCAPE:
            case KC_KEY_Q:
            {
                Device* device = DeviceManager::getDeviceManager()->getDevice();
                device->closeDevice();
                break;
            }
            default:
                break;
            }
        }
    }
}

int main(int argc, char *argv[])
{
    std::unique_ptr<DeviceManager> device_manager(new DeviceManager());
    bool success = device_manager->init();
    
    if (!success)
    {
        printf("Error: Couldn't create device manager.\n");
        return 1;
    }

    Device* device = device_manager->getDevice();
    device->setEventReceiver(onEvent);

    std::unique_ptr<FileManager> file_manager(new FileManager());
    success = file_manager->init();
    
    if (!success)
    {
        printf("Error: Couldn't create file manager.\n");
        return 1;
    }

    std::unique_ptr<TextureManager> texture_manager(new TextureManager());
    success = texture_manager->init();
    
    if (!success)
    {
        printf("Error: Couldn't create texture manager.\n");
        return 1;
    }

    std::unique_ptr<Camera> camera(new Camera(device->getWindowWidth(), 
                                               device->getWindowHeight()));

    std::unique_ptr<Renderer> renderer(new Renderer());
    success = renderer->init();
    
    if (!success)
    {
        printf("Error: Couldn't create renderer.\n");
        return 1;
    }

    std::unique_ptr<ModelManager> model_manager(new ModelManager());
    success = model_manager->init();
    
    if (!success)
    {
        printf("Error: Couldn't create model manager.\n");
        return 1;
    }

    VulkanContext* vulkan_context = device_manager->getVulkanContext();
    
    bool recreate_swapchain = false;
    bool quit = false;

    while (!quit)
    {
        bool quit = !device->processEvents();

        if (quit)
            break;

        unsigned int w = device->getWindowWidth();
        unsigned int h = device->getWindowHeight();

        if (w != vulkan_context->getDrawableWidth() ||
            h != vulkan_context->getDrawableHeight() ||
            recreate_swapchain)
        {
            recreate_swapchain = false;
            bool success = renderer->recreateSwapChain(w, h);

            if (!success)
            {
                printf("Error: Couldn't recreate swap chain");
                return 1;
            }
        }

        camera->update(w, h);

        bool success = renderer->drawFrame();
        
        if (!success)
        {
            recreate_swapchain = true;
        }
        
        device->sleep(10);
    }

    vulkan_context->waitIdle();

    return 0;
}

#ifdef ANDROID
void android_main(struct android_app* app) 
{
    app_dummy();
    
    DeviceAndroid::onCreate();
    
    g_android_app = app;
    main(0, {});
}
#endif
