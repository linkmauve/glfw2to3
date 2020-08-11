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

/* Extension support */

GLFWAPI int   GLFWAPIENTRY glfwExtensionSupported(const char *extension)
{
    return _glfw.glfwExtensionSupported(extension);
}

GLFWAPI void* GLFWAPIENTRY glfwGetProcAddress(const char *procname)
{
    return _glfw.glfwGetProcAddress(procname);
}

GLFWAPI void  GLFWAPIENTRY glfwGetGLVersion(int *major, int *minor, int *rev)
{
    if (!_glfw.window)
    {
        return;
    }

    const GLubyte* version = _glfw.glGetString(GL_VERSION);
    if (!version)
    {
        return;
    }

    /* Taken from GLFW 2.7.9 */
    const GLubyte* ptr = version;
    GLuint _major, _minor = 0, _rev = 0;
    for (_major = 0; *ptr >= '0' && *ptr <= '9'; ++ptr)
    {
        _major = 10 * _major + (*ptr - '0');
    }
    if (*ptr == '.')
    {
        ++ptr;
        for (_minor = 0; *ptr >= '0' && *ptr <= '9'; ++ptr)
        {
            _minor = 10 * _minor + (*ptr - '0');
        }
        if (*ptr == '.')
        {
            ++ptr;
            for (_rev = 0; *ptr >= '0' && *ptr <= '9'; ++ptr)
            {
                _rev = 10 * _rev + (*ptr - '0');
            }
        }
    }

    if (major)
    {
        *major = _major;
    }
    if (minor)
    {
        *minor = _minor;
    }
    if (rev)
    {
        *rev = _rev;
    }
}
