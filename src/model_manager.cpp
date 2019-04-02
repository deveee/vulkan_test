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
#include "model_manager.hpp"
#include "renderer.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <algorithm>
#include <cstring>

ModelManager* ModelManager::m_model_manager = nullptr;

ModelManager::ModelManager()
{
    m_model_manager = this;
}

ModelManager::~ModelManager()
{
    for (auto model : m_models)
    {
        delete model;
    }
}

bool ModelManager::init()
{
    Renderer* renderer = Renderer::getRenderer();
    
    FileManager* file_manager = FileManager::getFileManager();
    std::vector<std::string> assets_list = file_manager->getAssetsList();

    for (std::string name : assets_list)
    {
        std::string extension = file_manager->getExtension(name);
    
        if (extension != ".obj")
            continue;

        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;
        
        bool success = tinyobj::LoadObj(shapes, materials, err, name.c_str());
    
        if (!success)
            continue;
    
        for (unsigned int i = 0; i < shapes.size(); i++)
        {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
            std::string tex_name;
    
            tinyobj::mesh_t mesh = shapes[i].mesh;
    
            if (mesh.material_ids.size() > 0)
            {
                int material_id = mesh.material_ids[0];
    
                if (material_id > -1)
                {
                    tex_name = materials[material_id].diffuse_texname;
                }
            }
            
            for (auto index : mesh.indices)
            {
                Vertex vertex = {};

                vertex.pos = 
                {
                    mesh.positions[index * 3 + 0],
                    mesh.positions[index * 3 + 1],
                    mesh.positions[index * 3 + 2]
                };

                if (mesh.texcoords.size() > 0)
                {
                    vertex.tex_coord = 
                    {
                        mesh.texcoords[index * 2],
                        1.0f - mesh.texcoords[index * 2 + 1]
                    };
                }

                vertex.color = {1.0f, 1.0f, 1.0f};
                
                unsigned int vertex_id = vertices.size();
                
                for (unsigned int j = 0; j < vertices.size(); j++)
                {
                    if (vertex.pos == vertices[j].pos &&
                        vertex.color == vertices[j].color &&
                        vertex.tex_coord == vertices[j].tex_coord)
                    {
                        vertex_id = j;
                        break;
                    }
                }
                
                if (vertex_id == vertices.size())
                {
                    vertices.push_back(vertex);
                }

                indices.push_back(vertex_id);
            }
    
            Model* model = new Model(name, vertices, indices, 
                                     tex_name.empty() ? "white.png" : tex_name);
            m_models.push_back(model);
        }
    }
    
    bool success = renderer->createDescriptorPool(m_models.size());
    
    if (!success)
    {
        printf("Error: Couldn't create descriptor pool\n");
        return false;
    }
    
    for (Model* model : m_models)
    {
        success = model->init();
        
        if (!success)
        {
            printf("Error: Couldn't create model: %s\n", model->getName().c_str());
            return false;
        }
    }
    
    success = renderer->buildCommandBuffers(m_models);
    
    if (!success)
    {
        printf("Error: Couldn't build command buffers\n");
        return false;
    }

    return true;
}
