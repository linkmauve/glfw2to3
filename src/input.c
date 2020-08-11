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

/* Input handling */

GLFWAPI void GLFWAPIENTRY glfwPollEvents(void)
{
    _glfw.glfwPollEvents();
}

GLFWAPI void GLFWAPIENTRY glfwWaitEvents(void)
{
    _glfw.glfwWaitEvents();
}

GLFWAPI int  GLFWAPIENTRY glfwGetKey(int key)
{
    if (!_glfw.window)
    {
        return GLFW_RELEASE;
    }
    return _glfw.glfwGetKey(_glfw.window, key);
}

GLFWAPI int  GLFWAPIENTRY glfwGetMouseButton(int button)
{
    if (!_glfw.window)
    {
        return GLFW_RELEASE;
    }
    return _glfw.glfwGetMouseButton(_glfw.window, button);
}

GLFWAPI void GLFWAPIENTRY glfwGetMousePos(int *xpos, int *ypos)
{
    if (!_glfw.window)
    {
        return;
    }
    double xpos_double, ypos_double;
    _glfw.glfwGetCursorPos(_glfw.window, &xpos_double, &ypos_double);
    if (xpos)
    {
        *xpos = (int)xpos_double;
    }
    if (ypos)
    {
        *ypos = (int)ypos_double;
    }
}

GLFWAPI void GLFWAPIENTRY glfwSetMousePos(int xpos, int ypos)
{
    if (_glfw.window)
    {
        _glfw.glfwSetCursorPos(_glfw.window, (double)xpos, (double)ypos);
    }
}

GLFWAPI int  GLFWAPIENTRY glfwGetMouseWheel(void)
{
    // TODO: implement.
    return 0;
}

GLFWAPI void GLFWAPIENTRY glfwSetMouseWheel(int pos)
{
    fprintf(stderr, "Unimplemented glfwSetMouseWheel(%d)\n", pos);
}

static void key_cbfun3(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)window;
    (void)scancode;
    (void)mods;
    switch (key)
    {
    case 32:
        key = GLFW_KEY_SPACE;
        break;
    case 256:
        key = GLFW_KEY_ESC;
        break;
    case 257:
        key = GLFW_KEY_ENTER;
        break;
    case 262:
        key = GLFW_KEY_RIGHT;
        break;
    case 263:
        key = GLFW_KEY_LEFT;
        break;
    case 264:
        key = GLFW_KEY_DOWN;
        break;
    case 265:
        key = GLFW_KEY_UP;
        break;
    // TODO: add all of the other special keys.
    default:
        fprintf(stderr, "Unknown key %d, passing through\n", key);
        break;
    }
    _glfw.keyfun(key, action);
}

static void char_cbfun3(GLFWwindow* window, unsigned int codepoint)
{
    (void)window;
    _glfw.charfun((int)codepoint, GLFW_PRESS);
}

static void mousebutton_cbfun3(GLFWwindow* window, int button, int action, int mods)
{
    (void)window;
    (void)mods;
    _glfw.mousebuttonfun(button, action);
}

static void cursorpos_cbfun3(GLFWwindow* window, double xpos, double ypos)
{
    (void)window;
    _glfw.mouseposfun((int)xpos, (int)ypos);
}

static void scroll_cbfun3(GLFWwindow* window, double xoffset, double yoffset)
{
    (void)window;
    (void)xoffset;
    _glfw.mousewheelfun((int)yoffset);
}

GLFWAPI void GLFWAPIENTRY glfwSetKeyCallback(GLFWkeyfun cbfun)
{
    if (_glfw.window)
    {
        _glfw.keyfun = cbfun;
        _glfw.glfwSetKeyCallback(_glfw.window, key_cbfun3);
    }
}

GLFWAPI void GLFWAPIENTRY glfwSetCharCallback(GLFWcharfun cbfun)
{
    if (_glfw.window)
    {
        _glfw.charfun = cbfun;
        _glfw.glfwSetCharCallback(_glfw.window, char_cbfun3);
    }
}

GLFWAPI void GLFWAPIENTRY glfwSetMouseButtonCallback(GLFWmousebuttonfun cbfun)
{
    if (_glfw.window)
    {
        _glfw.mousebuttonfun = cbfun;
        _glfw.glfwSetMouseButtonCallback(_glfw.window, mousebutton_cbfun3);
    }
}

GLFWAPI void GLFWAPIENTRY glfwSetMousePosCallback(GLFWmouseposfun cbfun)
{
    if (_glfw.window)
    {
        _glfw.mouseposfun = cbfun;
        _glfw.glfwSetCursorPosCallback(_glfw.window, cursorpos_cbfun3);
    }
}

GLFWAPI void GLFWAPIENTRY glfwSetMouseWheelCallback(GLFWmousewheelfun cbfun)
{
    if (_glfw.window)
    {
        _glfw.mousewheelfun = cbfun;
        _glfw.glfwSetScrollCallback(_glfw.window, scroll_cbfun3);
    }
}
