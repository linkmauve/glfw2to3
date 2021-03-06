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

#include <stdio.h>

/* Enable/disable functions */

GLFWAPI void GLFWAPIENTRY glfwEnable(int token)
{
    switch (token)
    {
    case GLFW_KEY_REPEAT:
        // Nothing to do, already enabled in GLFW 3.
        break;
    default:
        fprintf(stderr, "Unsupported glfwEnable(0x%x)\n", token);
    }
}

GLFWAPI void GLFWAPIENTRY glfwDisable(int token)
{
    switch (token)
    {
    case GLFW_AUTO_POLL_EVENTS:
        // Nothing to do, already disabled in GLFW 3.
        break;
    default:
        fprintf(stderr, "Unsupported glfwDisable(0x%x)\n", token);
    }
}
