/*************************************************************************
 * GLFW 2to3 - www.glfw.org
 * A library easing porting from GLFW 2 to GLFW 3.x
 *------------------------------------------------------------------------
 * Copyright © 2020 Emmanuel Gil Peyrot <linkmauve@linkmauve.fr>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would
 *    be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 *************************************************************************/

#include "internal.h"

#include <string.h>

/* Joystick input */

GLFWAPI int GLFWAPIENTRY glfwGetJoystickParam(int joy, int param)
{
    int count;
    switch (param)
    {
    case GLFW_PRESENT:
        return _glfw.glfwJoystickPresent(joy);
    case GLFW_AXES:
        _glfw.glfwGetJoystickAxes(joy, &count);
        return count;
    case GLFW_BUTTONS:
        _glfw.glfwGetJoystickButtons(joy, &count);
        return count;
    default:
        return 0;
    }
}

GLFWAPI int GLFWAPIENTRY glfwGetJoystickPos(int joy, float *pos, int numaxes)
{
    int count;
    const float* joystick_axes = _glfw.glfwGetJoystickAxes(joy, &count);
    if (!joystick_axes)
    {
        return 0;
    }
    if (count > numaxes)
    {
        count = numaxes;
    }
    memcpy(pos, joystick_axes, count * sizeof(float));
    return count;
}

GLFWAPI int GLFWAPIENTRY glfwGetJoystickButtons(int joy, unsigned char *buttons, int numbuttons)
{
    int count;
    const unsigned char* joystick_buttons = _glfw.glfwGetJoystickButtons(joy, &count);
    if (!joystick_buttons)
    {
        return 0;
    }
    if (count > numbuttons)
    {
        count = numbuttons;
    }
    memcpy(buttons, joystick_buttons, count);
    return count;
}
