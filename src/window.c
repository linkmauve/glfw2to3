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

/* Window handling */

GLFWAPI int  GLFWAPIENTRY glfwOpenWindow(int width, int height, int redbits, int greenbits, int bluebits, int alphabits, int depthbits, int stencilbits, int mode)
{
    _glfw.glfwWindowHint(GLFW_RED_BITS, redbits);
    _glfw.glfwWindowHint(GLFW_GREEN_BITS, greenbits);
    _glfw.glfwWindowHint(GLFW_BLUE_BITS, bluebits);
    _glfw.glfwWindowHint(GLFW_ALPHA_BITS, alphabits);
    _glfw.glfwWindowHint(GLFW_DEPTH_BITS, depthbits);
    _glfw.glfwWindowHint(GLFW_STENCIL_BITS, stencilbits);

    GLFWmonitor* monitor;
    if (mode == GLFW_WINDOW)
    {
        monitor = NULL;
    }
    else
    {
        monitor = _glfw.glfwGetPrimaryMonitor();
    }

    _glfw.window = _glfw.glfwCreateWindow(width, height, "GLFW2to3", monitor, NULL);
    if (!_glfw.window)
    {
        return GL_FALSE;
    }

#define GETPROCADDRESS(sym) do { \
    _glfw.sym = (PFN_##sym)glfwGetProcAddress(#sym); \
    if (!_glfw.sym) \
    { \
        return GL_FALSE; \
    } \
} while (0)

    GETPROCADDRESS(glGetString);
    GETPROCADDRESS(glPixelStorei);
    GETPROCADDRESS(glGetTexParameteriv);
    GETPROCADDRESS(glTexParameteri);
    GETPROCADDRESS(glTexImage2D);

#undef GETPROCADDRESS

    _glfw.glfwMakeContextCurrent(_glfw.window);

    return GL_TRUE;
}

GLFWAPI void GLFWAPIENTRY glfwOpenWindowHint(int target, int hint)
{
    switch (target)
    {
    case GLFW_WINDOW_NO_RESIZE:
        _glfw.glfwWindowHint(0x00020003, !hint);
        break;
    default:
        fprintf(stderr, "Unsupported glfwOpenWindowHint(0x%x, 0x%x)\n", target, hint);
        break;
    }
}

GLFWAPI void GLFWAPIENTRY glfwCloseWindow(void)
{
    if (_glfw.window)
    {
        _glfw.glfwDestroyWindow(_glfw.window);
        _glfw.window = NULL;
    }
}

GLFWAPI void GLFWAPIENTRY glfwSetWindowTitle(const char *title)
{
    if (_glfw.window)
    {
        _glfw.glfwSetWindowTitle(_glfw.window, title);
    }
}

GLFWAPI void GLFWAPIENTRY glfwGetWindowSize(int *width, int *height)
{
    if (_glfw.window)
    {
        _glfw.glfwGetWindowSize(_glfw.window, width, height);
    }
}

GLFWAPI void GLFWAPIENTRY glfwSetWindowSize(int width, int height)
{
    if (_glfw.window)
    {
        _glfw.glfwSetWindowSize(_glfw.window, width, height);
    }
}

GLFWAPI void GLFWAPIENTRY glfwSetWindowPos(int x, int y)
{
    if (_glfw.window)
    {
        _glfw.glfwSetWindowPos(_glfw.window, x, y);
    }
}

GLFWAPI void GLFWAPIENTRY glfwIconifyWindow(void)
{
    if (_glfw.window)
    {
        _glfw.glfwIconifyWindow(_glfw.window);
    }
}

GLFWAPI void GLFWAPIENTRY glfwRestoreWindow(void)
{
    if (_glfw.window)
    {
        _glfw.glfwRestoreWindow(_glfw.window);
    }
}

GLFWAPI void GLFWAPIENTRY glfwSwapBuffers(void)
{
    if (_glfw.window)
    {
        _glfw.glfwSwapBuffers(_glfw.window);
    }
}

GLFWAPI void GLFWAPIENTRY glfwSwapInterval(int interval)
{
    _glfw.glfwSwapInterval(interval);
}

GLFWAPI int  GLFWAPIENTRY glfwGetWindowParam(int param)
{
    if (!_glfw.window)
    {
        return GL_FALSE;
    }
    switch (param)
    {
    case GLFW_OPENED:
        return !!_glfw.window;
    case GLFW_ACTIVE:
        return _glfw.glfwGetWindowAttrib(_glfw.window, 0x00020001);
    case GLFW_ICONIFIED:
        return _glfw.glfwGetWindowAttrib(_glfw.window, 0x00020002);
    default:
        fprintf(stderr, "Unsupported glfwGetWindowParam(0x%x)\n", param);
        return GL_FALSE;
    }
}

static void size_cbfun3(GLFWwindow* window, int width, int height)
{
    (void)window;
    _glfw.windowsizefun(width, height);
}

static int close_cbfun3(GLFWwindow* window)
{
    (void)window;
    return _glfw.closefun();
}

static void refresh_cbfun3(GLFWwindow* window)
{
    (void)window;
    _glfw.refreshfun();
}

GLFWAPI void GLFWAPIENTRY glfwSetWindowSizeCallback(GLFWwindowsizefun cbfun)
{
    if (_glfw.window)
    {
        _glfw.windowsizefun = cbfun;
        _glfw.glfwSetWindowSizeCallback(_glfw.window, size_cbfun3);
    }
}

GLFWAPI void GLFWAPIENTRY glfwSetWindowCloseCallback(GLFWwindowclosefun cbfun)
{
    if (_glfw.window)
    {
        _glfw.closefun = cbfun;
        _glfw.glfwSetWindowCloseCallback(_glfw.window, close_cbfun3);
    }
}

GLFWAPI void GLFWAPIENTRY glfwSetWindowRefreshCallback(GLFWwindowrefreshfun cbfun)
{
    if (_glfw.window)
    {
        _glfw.refreshfun = cbfun;
        _glfw.glfwSetWindowRefreshCallback(_glfw.window, refresh_cbfun3);
    }
}
