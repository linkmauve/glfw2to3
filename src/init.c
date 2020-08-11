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

#include <dlfcn.h>

_GLFWlibrary _glfw = { 0 };

/* GLFW initialization, termination and version querying */

GLFWAPI int  GLFWAPIENTRY glfwInit(void)
{
    if (!_glfw.handle)
    {
        _glfw.handle = dlopen("libglfw.so.3", RTLD_LAZY);
        if (!_glfw.handle)
        {
            return GL_FALSE;
        }
    }

#define DLSYM(sym) do { \
    _glfw.sym = (PFN_##sym)dlsym(_glfw.handle, #sym); \
    if (!_glfw.sym) \
    { \
        return GL_FALSE; \
    } \
} while (0)

    DLSYM(glfwInit);
    DLSYM(glfwTerminate);
    DLSYM(glfwCreateWindow);
    DLSYM(glfwDestroyWindow);
    DLSYM(glfwIconifyWindow);
    DLSYM(glfwRestoreWindow);
    DLSYM(glfwMakeContextCurrent);
    DLSYM(glfwWindowHint);
    DLSYM(glfwSetWindowTitle);
    DLSYM(glfwSetWindowSizeCallback);
    DLSYM(glfwSetWindowCloseCallback);
    DLSYM(glfwSetWindowRefreshCallback);
    DLSYM(glfwSetKeyCallback);
    DLSYM(glfwSetCharCallback);
    DLSYM(glfwGetMouseButton);
    DLSYM(glfwSetMouseButtonCallback);
    DLSYM(glfwSetCursorPosCallback);
    DLSYM(glfwSetScrollCallback);
    DLSYM(glfwGetWindowSize);
    DLSYM(glfwSetWindowSize);
    DLSYM(glfwGetWindowAttrib);
    DLSYM(glfwSetWindowPos);
    DLSYM(glfwSwapBuffers);
    DLSYM(glfwSwapInterval);
    DLSYM(glfwExtensionSupported);
    DLSYM(glfwGetProcAddress);
    DLSYM(glfwGetPrimaryMonitor);
    DLSYM(glfwGetVideoModes);
    DLSYM(glfwGetVideoMode);
    DLSYM(glfwPollEvents);
    DLSYM(glfwWaitEvents);
    DLSYM(glfwGetKey);
    DLSYM(glfwGetCursorPos);
    DLSYM(glfwSetCursorPos);

#undef DLSYM

    return _glfw.glfwInit();
}

GLFWAPI void GLFWAPIENTRY glfwTerminate(void)
{
    if (_glfw.handle)
    {
        dlclose(_glfw.handle);
        _glfw.handle = NULL;
    }
}

GLFWAPI void GLFWAPIENTRY glfwGetVersion(int *major, int *minor, int *rev)
{
    if (major)
    {
        *major = 2;
    }
    if (minor)
    {
        *minor = 7;
    }
    if (rev)
    {
        *rev = 10;
    }
}
