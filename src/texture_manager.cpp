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
#include "image_loader.hpp"
#include "texture_manager.hpp"

#include <algorithm>
#include <cstring>

TextureManager* TextureManager::m_texture_manager = nullptr;

TextureManager::TextureManager()
{
    m_texture_manager = this;
}

TextureManager::~TextureManager()
{
    for (auto texture : m_textures)
    {
        if (texture.second == nullptr)
            continue;
            
        delete texture.second->vulkan_image;
        delete texture.second;
    }
}

bool TextureManager::init()
{
    loadTextures();

    return true;
}

void TextureManager::loadTextures()
{
    FileManager* file_manager = FileManager::getFileManager();
    std::vector<std::string> assets_list = file_manager->getAssetsList();

    for (std::string name : assets_list)
    {
        Image* image = ImageLoader::loadImage(name);

        if (image == nullptr)
            continue;
        
        if (image->channels < 3 || image->channels > 4)
        {
            printf("Warning: Couldn't load texture: %s\n", name.c_str());
            ImageLoader::closeImage(image);
            continue;
        }
            
        unsigned char* data = nullptr;
        
        if (image->channels == 3)
        {
            data = new unsigned char[image->width * image->height * 4];
            convertToRGBA(image->data, image->width * image->height * 3, data);
        }
        else
        {
            data = image->data;
        }
        
        Texture* texture = createTexture(image->width, image->height, 4, data);

        if (texture)
        {
            m_textures[name] = texture;
        }
        else
        {
            printf("Warning: Couldn't load texture: %s\n", name.c_str());
        }
        
        if (image->channels == 3)
        {
            delete[] data;
        }

        ImageLoader::closeImage(image);
    }
}

Texture* TextureManager::createTexture(int width, int height, int channels,
                                       const void* data)
{
    VulkanImage* image = new VulkanImage(VK_FORMAT_R8G8B8A8_UNORM, width, height);

    bool success = image->createTextureImage(data, channels);

    if (!success)
    {
        delete image;
        return nullptr;
    }

    success = image->createImageView(VK_IMAGE_ASPECT_COLOR_BIT);

    if (!success)
    {
        delete image;
        return nullptr;
    }

    success = image->createSampler();

    if (!success)
    {
        delete image;
        return nullptr;
    }

    Texture* texture = new Texture();
    texture->width = width;
    texture->height = height;
    texture->channels = channels;
    texture->vulkan_image = image;

    return texture;
}

void TextureManager::convertToRGBA(unsigned char* src, unsigned int src_length,
                                   unsigned char* dst)
{
    unsigned int pixels = src_length / 3;

    for (unsigned int i = 0; i < pixels; i++)
    {
        dst[i*4]   = src[i*3];
        dst[i*4+1] = src[i*3+1];
        dst[i*4+2] = src[i*3+2];
        dst[i*4+3] = 255;
    }
}
