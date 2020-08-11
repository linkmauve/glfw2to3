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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

/* Threading support */

// TODO: use a mutex to protect this variable!
static thrd_t* threads = NULL;
static int allocated = 0;
static int size = 0;

typedef struct fat_arg
{
    GLFWthreadfun fun;
    void* arg;
} fat_arg;

static int start_thread(void* arg)
{
    fat_arg* fat_arg = arg;
    fat_arg->fun(fat_arg->arg);
    return 0;
}

GLFWAPI GLFWthread GLFWAPIENTRY glfwCreateThread(GLFWthreadfun fun, void *arg)
{
    if (allocated == 0)
    {
        threads = malloc(sizeof(thrd_t));
        allocated = 1;
    }
    else if (size + 1 > allocated)
    {
        threads = realloc(threads, 2 * allocated * sizeof(thrd_t));
        for (int i = 0; i < allocated; ++i)
        {
            threads[allocated + i] = ~0UL;
        }
        allocated *= 2;
    }
    // TODO: can this be put on the stack?
    fat_arg fat_arg = {
        fun,
        arg,
    };
    // TODO: be smarter with reuse of IDs.
    if (thrd_create(&threads[size + 1], start_thread, &fat_arg) != thrd_success)
    {
        return -1;
    }
    return ++size;
}

GLFWAPI void GLFWAPIENTRY glfwDestroyThread(GLFWthread ID)
{
    // TODO: figure out how and whether to implement.
    fprintf(stderr, "glfwDestroyThread(%d), dangerous function left unimplemented.\n", ID);
    threads[ID] = ~0UL;
}

GLFWAPI int  GLFWAPIENTRY glfwWaitThread(GLFWthread ID, int waitmode)
{
    if (threads[ID] == ~0UL)
    {
        return GL_TRUE;
    }
    if (waitmode == GLFW_WAIT)
    {
        thrd_join(threads[ID], NULL);
        return GL_TRUE;
    }
    else // if (waitmore == GLFW_NOWAIT)
    {
        // TODO: must return GL_TRUE if the thread is running.
        return GL_FALSE;
    }
}

GLFWAPI GLFWthread GLFWAPIENTRY glfwGetThreadID(void)
{
    thrd_t thrd = thrd_current();
    for (int i = 0; i < size; ++i)
    {
        if (threads[i] == thrd)
        {
            return i;
        }
    }
    return -1;
}

GLFWAPI GLFWmutex GLFWAPIENTRY glfwCreateMutex(void)
{
    mtx_t* mutex = malloc(sizeof(mtx_t));
    if (mtx_init(mutex, mtx_plain) != thrd_success)
    {
        return NULL;
    }
    return mutex;
}

GLFWAPI void GLFWAPIENTRY glfwDestroyMutex(GLFWmutex mutex)
{
    mtx_destroy(mutex);
    free(mutex);
}

GLFWAPI void GLFWAPIENTRY glfwLockMutex(GLFWmutex mutex)
{
    mtx_lock(mutex);
}

GLFWAPI void GLFWAPIENTRY glfwUnlockMutex(GLFWmutex mutex)
{
    mtx_unlock(mutex);
}

GLFWAPI GLFWcond GLFWAPIENTRY glfwCreateCond(void)
{
    cnd_t* cond = malloc(sizeof(cnd_t));
    if (cnd_init(cond) != thrd_success)
    {
        return NULL;
    }
    return cond;
}

GLFWAPI void GLFWAPIENTRY glfwDestroyCond(GLFWcond cond)
{
    cnd_destroy(cond);
    free(cond);
}

GLFWAPI void GLFWAPIENTRY glfwWaitCond(GLFWcond cond, GLFWmutex mutex, double timeout)
{
    struct timespec tp;
    double sec;
    double nsec = modf(timeout, &sec) * 1000000000.0;
    tp.tv_sec = (time_t)sec;
    tp.tv_nsec = (long)nsec;
    cnd_timedwait(cond, mutex, &tp);
}

GLFWAPI void GLFWAPIENTRY glfwSignalCond(GLFWcond cond)
{
    cnd_signal(cond);
}

GLFWAPI void GLFWAPIENTRY glfwBroadcastCond(GLFWcond cond)
{
    cnd_broadcast(cond);
}

GLFWAPI int  GLFWAPIENTRY glfwGetNumberOfProcessors(void)
{
    // TODO: make this more portable.
    return sysconf(_SC_NPROCESSORS_ONLN);
}
