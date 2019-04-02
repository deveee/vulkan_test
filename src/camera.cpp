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

#include <cmath>

#include "camera.hpp"

Camera* Camera::m_camera = nullptr;

Camera::Camera(unsigned int width, unsigned int height)
{
    m_camera = this;

    m_position = glm::vec3(0.0f, 5.0f, 0.0f);
    m_direction = glm::vec3(0.0f, 0.0f, 0.0f);
    m_right = glm::vec3(0, 0, 0);
    m_up = glm::vec3(0.0f, 1.0f, 0.0f);
    m_horizontal_angle = M_PI/2;
    m_vertical_angle = 0;
    
    m_original_width = width;
    m_original_height = height;

    rotate(0, 0);
    update(width, height);
}

Camera::~Camera()
{
}

void Camera::rotate(float horizontal, float vertical)
{
    m_horizontal_angle += horizontal;
    m_vertical_angle += vertical;

    m_direction.x = cosf(m_vertical_angle) * sinf(m_horizontal_angle);
    m_direction.y = sinf(m_vertical_angle);
    m_direction.z = cosf(m_vertical_angle) * cosf(m_horizontal_angle);

    m_right.x = sinf(m_horizontal_angle - M_PI/2.0);
    m_right.y = 0.0;
    m_right.z = cosf(m_horizontal_angle - M_PI/2.0);

    m_up = glm::cross(m_right, m_direction);
}

void Camera::moveForward(float value)
{
    m_position.x += m_direction.x * value;
    m_position.y += m_direction.y * value;
    m_position.z += m_direction.z * value;
}

void Camera::moveBackward(float value)
{
    moveForward(value * -1.0);
}

void Camera::moveRight(float value)
{
    m_position.x += m_right.x * value;
    m_position.y += m_right.y * value;
    m_position.z += m_right.z * value;
}

void Camera::moveLeft(float value)
{
    moveRight(value * -1.0);
}

void Camera::update(unsigned int width, unsigned int height)
{
    float ratio = ((float)width * m_original_height) / 
                  ((float)height * m_original_width);

    m_proj_matrix = glm::perspective(glm::radians(50.0f), ratio, 0.1f, 100.0f);
    m_proj_matrix[1][1] *= -1;
    
    m_view_matrix = glm::lookAt(glm::vec3(m_position.x,
                                          m_position.y,
                                          m_position.z),
                                glm::vec3(m_position.x + m_direction.x,
                                          m_position.y + m_direction.y,
                                          m_position.z + m_direction.z),
                                glm::vec3(m_up.x,
                                          m_up.y,
                                          m_up.z));
}
