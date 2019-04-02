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

#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP

#include "vulkan_image.hpp"

#include <map>
#include <string>

struct Texture
{
    VulkanImage* vulkan_image;
    unsigned int width;
    unsigned int height;
    unsigned int channels;
};

class TextureManager
{
private:
    std::map<std::string, Texture*> m_textures;
    static TextureManager* m_texture_manager;

    void loadTextures();
    void convertToRGBA(unsigned char* src, unsigned int src_length,
                       unsigned char* dst);

public:
    TextureManager();
    ~TextureManager();

    bool init();
    Texture* createTexture(int width, int height, int channels,
                           const void* data);
    Texture* getTexture(std::string name) {return m_textures[name];}

    static TextureManager* getTextureManager() {return m_texture_manager;}
};

#endif
