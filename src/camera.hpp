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

#ifndef CAMERA_HPP
#define CAMERA_HPP

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera
{
private:
    glm::mat4 m_view_matrix;
    glm::mat4 m_proj_matrix;

    glm::vec3 m_position;
    glm::vec3 m_direction;
    glm::vec3 m_right;
    glm::vec3 m_up;
    float m_horizontal_angle;
    float m_vertical_angle;
    
    unsigned int m_original_width;
    unsigned int m_original_height;

    static Camera* m_camera;

public:
    Camera(unsigned int width, unsigned int height);
    ~Camera();

    void rotate(float x, float y);
    void moveForward(float amount);
    void moveBackward(float amount);
    void moveLeft(float amount);
    void moveRight(float amount);

    void update(unsigned int width, unsigned int height);

    glm::vec3 getCameraPos() {return m_position;}
    glm::mat4 getViewMatrix() {return m_view_matrix;}
    glm::mat4 getProjMatrix() {return m_proj_matrix;}

    static Camera* getCamera() {return m_camera;}
};


#endif
