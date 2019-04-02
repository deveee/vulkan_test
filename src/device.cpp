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

#include "device.hpp"

#include <cmath>

Device::Device()
{
    m_window_width = 0;
    m_window_height = 0;
    m_event_receiver = nullptr;
}

void Device::sleep(unsigned int time_ms)
{
    struct timespec ts;
    ts.tv_sec = (time_t)(time_ms / 1000);
    ts.tv_nsec = (long)(time_ms % 1000) * 1000000;

    nanosleep(&ts, nullptr);
}

unsigned long Device::getMicroTickCount()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    return (unsigned long)(now.tv_sec) * 1000000 + now.tv_nsec / 1000;
}

void Device::sendEvent(Event event)
{
    if (m_event_receiver == nullptr)
        return;

    m_event_receiver(event);
}

MouseEventType Device::checkMouseClick(MouseEvent event)
{
    if (event.type < ME_LEFT_PRESSED || event.type > ME_RIGHT_RELEASED)
        return ME_COUNT;

    const unsigned int DOUBLE_CLICK_TIME = 500000;
    const int MAX_MOUSEMOVE = 3;

    MouseEventType event_type = ME_COUNT;
    unsigned long click_time = getMicroTickCount();

    int button = 0;

    switch (event.type)
    {
    case ME_LEFT_PRESSED:
    case ME_LEFT_RELEASED:
        button = 1;
        break;
    case ME_MIDDLE_PRESSED:
    case ME_MIDDLE_RELEASED:
        button = 2;
        break;
    case ME_RIGHT_PRESSED:
    case ME_RIGHT_RELEASED:
        button = 3;
        break;
    default:
        break;
    }

    if (click_time - m_mouse_clicks.last_click_time < DOUBLE_CLICK_TIME &&
        std::abs(m_mouse_clicks.last_pos_x - event.x) <= MAX_MOUSEMOVE &&
        std::abs(m_mouse_clicks.last_pos_y - event.y) <= MAX_MOUSEMOVE &&
        m_mouse_clicks.last_button == button && m_mouse_clicks.count < 2)
    {
        bool pressed = (event.type == ME_LEFT_PRESSED ||
                        event.type == ME_MIDDLE_PRESSED ||
                        event.type == ME_RIGHT_PRESSED);

        if (!pressed && m_mouse_clicks.count == 0)
        {
            m_mouse_clicks.count++;
            event_type = (MouseEventType)(ME_LEFT_CLICK
                                            + event.type - ME_LEFT_RELEASED);
        }
        else if (pressed && m_mouse_clicks.count == 1)
        {
            m_mouse_clicks.count++;
            event_type = (MouseEventType)(ME_LEFT_DOUBLE_CLICK
                                            + event.type - ME_LEFT_PRESSED);
        }
    }
    else
    {
        m_mouse_clicks.count = 0;
    }

    m_mouse_clicks.last_button = button;
    m_mouse_clicks.last_click_time = click_time;
    m_mouse_clicks.last_pos_x = event.x;
    m_mouse_clicks.last_pos_y = event.y;

    return event_type;
}
