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

#ifndef MODEL_MANAGER_HPP
#define MODEL_MANAGER_HPP

#include "model.hpp"

#include <string>
#include <vector>


class ModelManager
{
private:
    std::vector<Model*> m_models;
    static ModelManager* m_model_manager;

public:
    ModelManager();
    ~ModelManager();

    bool init();
    const std::vector<Model*>& getModels() {return m_models;}

    static ModelManager* getModelManager() {return m_model_manager;}
};

#endif
