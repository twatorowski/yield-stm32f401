/**
 * @file lock.c
 *
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-04-05
 *
 * @brief Semaphore lock
 */

#include "err.h"
#include "sys/sem.h"
#include "sys/time.h"
#include "sys/yield.h"

/* lock resource */
err_t Sem_Lock(sem_t *sem, dtime_t timeout)
{
    /* get current time */
    time_t ts = time(0); int task_id = Yield_GetTaskID();

    /* loop as long as */
    while (*sem != SEM_RELEASED) {
        /* semaphore belongs to our task? */
        if (*sem == task_id)
            break;
        /* check for timeout */
        if (timeout && dtime(time(0), ts) > timeout)
            return ETIMEOUT;
        /* yield while waiting */
        Yield();
    }
    /* locked! */
    *sem = task_id;
    /* report success */
    return EOK;
}

/* lock multiple semaphores */
err_t Sem_LockMultiple(sem_t **sem_list, dtime_t timeout)
{
    /* get current time */
    time_t ts = time(0); sem_t **s;
    /* get current task id */
    int task_id = Yield_GetTaskID();

    /* wait for all semaphores to be locable */
    while (!timeout || dtime(time(0), ts) < timeout) {
        /* lock as many as possible */
        for (s = sem_list; *s && (*(*s) == SEM_RELEASED || *(*s) == task_id); s++)
            *(*s) = task_id;
        /* all were locked? */
        if (*s == 0)
            return EOK;
        /* some were locked, release them */
        for (; s != sem_list; s--)
            *(*(s-1)) = SEM_RELEASED;
        /* yield while waiting */
        Yield();
    }

    /* we've exited the loop, so this must be due to timeout  */
    return ETIMEOUT;
}

/* release underlying resource */
err_t Sem_Release(sem_t *sem)
{
    /* release*/
    *sem = SEM_RELEASED;
    /* report status */
    return EOK;
}

/* release multiple semaphores */
err_t Sem_ReleaseMultiple(sem_t **sem_list)
{
    /* some were locked, release them */
    for (sem_t **s = sem_list; *s; s++)
        *(*(s)) = SEM_RELEASED;

    /* return status */
    return EOK;
}

