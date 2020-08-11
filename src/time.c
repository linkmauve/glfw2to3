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

#include <threads.h>
#include <math.h>

static uint64_t getTime(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t) ts.tv_sec * (uint64_t) 1000000000 + (uint64_t) ts.tv_nsec;
}

void _glfwInitTimer(void)
{
    _glfw.timer_base = getTime();
}

GLFWAPI double GLFWAPIENTRY glfwGetTime(void)
{
    return (double) (getTime() - _glfw.timer_base) * 1e-9;
}

GLFWAPI void   GLFWAPIENTRY glfwSetTime(double time)
{
    _glfw.timer_base = getTime() - (uint64_t) (time / 1e-9);
}

GLFWAPI void   GLFWAPIENTRY glfwSleep(double time)
{
    if (time > 0.0)
    {
        struct timespec dur;
        double sec;
        double nsec = modf(time, &sec) * 1000000000.0;
        dur.tv_sec = (time_t)sec;
        dur.tv_nsec = (long)nsec;
        // TODO: maybe use a loop here to sleep for the remaining time on interrupt?
        thrd_sleep(&dur, NULL);
    }
    else
    {
        thrd_yield();
    }
}
