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

/* Video mode functions */

GLFWAPI int  GLFWAPIENTRY glfwGetVideoModes(GLFWvidmode *list, int maxcount)
{
    GLFWmonitor* monitor = _glfw.glfwGetPrimaryMonitor();
    int count;
    const vidmode3* modes = _glfw.glfwGetVideoModes(monitor, &count);
    if (count > maxcount)
    {
        count = maxcount;
    }
    for (int i = 0; i < count; ++i)
    {
        memcpy(list + i, modes + i, sizeof(GLFWvidmode));
    }
    return count;
}

GLFWAPI void GLFWAPIENTRY glfwGetDesktopMode(GLFWvidmode *mode)
{
    GLFWmonitor* monitor = _glfw.glfwGetPrimaryMonitor();
    const vidmode3* vidmode3 = _glfw.glfwGetVideoMode(monitor);
    memcpy(mode, &vidmode3, sizeof(GLFWvidmode));
}
