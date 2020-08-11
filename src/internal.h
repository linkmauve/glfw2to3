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

#include "GL/glfw.h"

typedef struct GLFWwindow GLFWwindow;
typedef struct GLFWmonitor GLFWmonitor;

typedef struct vidmode3 {
    GLFWvidmode vidmode;
    int refreshRate;
} vidmode3;

typedef int (* PFN_glfwInit)(void);
typedef void (* PFN_glfwTerminate)(void);
typedef GLFWwindow* (* PFN_glfwCreateWindow)(int, int, const char*, GLFWmonitor*, GLFWwindow*);
typedef void (* PFN_glfwDestroyWindow)(GLFWwindow*);
typedef void (* PFN_glfwIconifyWindow)(GLFWwindow*);
typedef void (* PFN_glfwRestoreWindow)(GLFWwindow*);
typedef void (* PFN_glfwMakeContextCurrent)(GLFWwindow*);
typedef void (* PFN_glfwWindowHint)(int, int);
typedef void (* PFN_glfwSetWindowTitle)(GLFWwindow*, const char*);
typedef void (* PFN_glfwSetWindowPos)(GLFWwindow*, int, int);
typedef void (* PFN_glfwGetWindowSize)(GLFWwindow*, int*, int*);
typedef void (* PFN_glfwSetWindowSize)(GLFWwindow*, int, int);
typedef int (* PFN_glfwGetWindowAttrib)(GLFWwindow*, int);
typedef void (* PFN_glfwSetWindowSizeCallback)(GLFWwindow*, void (*)(GLFWwindow*, int, int));
typedef void (* PFN_glfwSetWindowCloseCallback)(GLFWwindow*, int (*)(GLFWwindow*));
typedef void (* PFN_glfwSetWindowRefreshCallback)(GLFWwindow*, void (*)(GLFWwindow*));
typedef void (* PFN_glfwSetKeyCallback)(GLFWwindow*, void (*)(GLFWwindow*, int, int, int, int));
typedef void (* PFN_glfwSetCharCallback)(GLFWwindow*, void (*)(GLFWwindow*, unsigned int));
typedef int (* PFN_glfwGetMouseButton)(GLFWwindow*, int);
typedef void (* PFN_glfwSetMouseButtonCallback)(GLFWwindow*, void (*)(GLFWwindow*, int, int, int));
typedef void (* PFN_glfwSetCursorPosCallback)(GLFWwindow*, void (*)(GLFWwindow*, double, double));
typedef void (* PFN_glfwSetScrollCallback)(GLFWwindow*, void (*)(GLFWwindow*, double, double));
typedef void (* PFN_glfwSwapBuffers)(GLFWwindow*);
typedef void (* PFN_glfwSwapInterval)(int);
typedef int (* PFN_glfwExtensionSupported)(const char*);
typedef void* (* PFN_glfwGetProcAddress)(const char*);
typedef GLFWmonitor* (* PFN_glfwGetPrimaryMonitor)(void);
typedef const vidmode3* (* PFN_glfwGetVideoModes)(GLFWmonitor*, int*);
typedef const vidmode3* (* PFN_glfwGetVideoMode)(GLFWmonitor*);
typedef void (* PFN_glfwPollEvents)(void);
typedef void (* PFN_glfwWaitEvents)(void);
typedef int (* PFN_glfwGetKey)(GLFWwindow*, int);
typedef void (* PFN_glfwGetCursorPos)(GLFWwindow*, double*, double*);
typedef void (* PFN_glfwSetCursorPos)(GLFWwindow*, double, double);
typedef int (* PFN_glfwJoystickPresent)(int);
typedef const float* (* PFN_glfwGetJoystickAxes)(int, int* count);
typedef const unsigned char* (* PFN_glfwGetJoystickButtons)(int, int* count);

typedef const GLubyte* (* PFN_glGetString)(GLenum);
typedef void (* PFN_glPixelStorei)(GLenum, GLint);
typedef void (* PFN_glGetTexParameteriv)(GLenum, GLenum, GLint*);
typedef void (* PFN_glGetIntegerv)(GLenum, GLint*);
typedef void (* PFN_glTexParameteri)(GLenum, GLenum, GLint);
typedef void (* PFN_glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid*);

#define GL_FALSE 0
#define GL_TRUE 1

typedef struct _GLFWlibrary {
    void* handle;
    void* gl_handle;

    GLFWwindow* window;
    void (*windowsizefun)(int, int);
    int (*closefun)(void);
    void (*refreshfun)(void);
    void (*keyfun)(int, int);
    void (*charfun)(int, int);
    void (*mousebuttonfun)(int, int);
    void (*mouseposfun)(int, int);
    void (*mousewheelfun)(int);

    PFN_glfwInit glfwInit;
    PFN_glfwTerminate glfwTerminate;
    PFN_glfwCreateWindow glfwCreateWindow;
    PFN_glfwDestroyWindow glfwDestroyWindow;
    PFN_glfwIconifyWindow glfwIconifyWindow;
    PFN_glfwRestoreWindow glfwRestoreWindow;
    PFN_glfwMakeContextCurrent glfwMakeContextCurrent;
    PFN_glfwWindowHint glfwWindowHint;
    PFN_glfwSetWindowTitle glfwSetWindowTitle;
    PFN_glfwSetWindowSizeCallback glfwSetWindowSizeCallback;
    PFN_glfwSetWindowCloseCallback glfwSetWindowCloseCallback;
    PFN_glfwSetWindowRefreshCallback glfwSetWindowRefreshCallback;
    PFN_glfwSetKeyCallback glfwSetKeyCallback;
    PFN_glfwSetCharCallback glfwSetCharCallback;
    PFN_glfwGetMouseButton glfwGetMouseButton;
    PFN_glfwSetMouseButtonCallback glfwSetMouseButtonCallback;
    PFN_glfwSetCursorPosCallback glfwSetCursorPosCallback;
    PFN_glfwSetScrollCallback glfwSetScrollCallback;
    PFN_glfwGetWindowSize glfwGetWindowSize;
    PFN_glfwSetWindowSize glfwSetWindowSize;
    PFN_glfwGetWindowAttrib glfwGetWindowAttrib;
    PFN_glfwSetWindowPos glfwSetWindowPos;
    PFN_glfwSwapBuffers glfwSwapBuffers;
    PFN_glfwSwapInterval glfwSwapInterval;
    PFN_glfwExtensionSupported glfwExtensionSupported;
    PFN_glfwGetProcAddress glfwGetProcAddress;
    PFN_glfwGetPrimaryMonitor glfwGetPrimaryMonitor;
    PFN_glfwGetVideoModes glfwGetVideoModes;
    PFN_glfwGetVideoMode glfwGetVideoMode;
    PFN_glfwPollEvents glfwPollEvents;
    PFN_glfwWaitEvents glfwWaitEvents;
    PFN_glfwGetKey glfwGetKey;
    PFN_glfwGetCursorPos glfwGetCursorPos;
    PFN_glfwSetCursorPos glfwSetCursorPos;
    PFN_glfwJoystickPresent glfwJoystickPresent;
    PFN_glfwGetJoystickAxes glfwGetJoystickAxes;
    PFN_glfwGetJoystickButtons glfwGetJoystickButtons;

    PFN_glGetString         glGetString;
    PFN_glPixelStorei       glPixelStorei;
    PFN_glGetTexParameteriv glGetTexParameteriv;
    PFN_glGetIntegerv       glGetIntegerv;
    PFN_glTexParameteri     glTexParameteri;
    PFN_glTexImage2D        glTexImage2D;

    uint64_t timer_base;
} _GLFWlibrary;

extern _GLFWlibrary _glfw;
