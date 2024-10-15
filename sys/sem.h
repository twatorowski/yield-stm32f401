/**
 * @file lock.h
 *
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-04-05
 *
 * @brief Semaphore lock
 */

#ifndef _SYS_SEM_H
#define _SYS_SEM_H

#include "err.h"
#include "sys/time.h"

/** semaphore lock typedef */
typedef enum sem { SEM_RELEASED, SEM_LOCKED } sem_t;

/** macro for doing an operation with a semaphore */
#define with_sem(sem)                                                       \
    for (int __once = Sem_Lock(sem, 0) + 1;                                 \
        __once > 0; Sem_Release(sem), __once = 0)


/** macro for doing an operation with a semaphore (with timeout) */
#define with_sem_to(sem, timeout, ec)                                       \
    for (int __once = (ec = Sem_Lock(sem, timeout)) + 1;                    \
        __once > 0 && ec >= 0; Sem_Release(sem), __once = 0)


/** macro for doing an operation with collection of semaphores */
#define with_sems(sem_list)                                                 \
    for (int __once = Sem_LockMultiple(sem_list, 0) + 1;                    \
        __once  > 0; Sem_ReleaseMultiple(sem_list), __once = 0)

/** macro for doing an operation with collection of semaphores (with timeout)*/
#define with_sems_to(sem_list, timeout, ec)                                 \
    for (int __once = (ec = Sem_LockMultiple(sem_list, timeout)) + 1;       \
        __once  > 0 && ec >= 0; Sem_ReleaseMultiple(sem_list), __once = 0)

/**
 * @brief Lock the resource
 *
 * @param sem lock to acquire
 * @param timeout locking timeout
 *
 * @return err_t EOK or ETIMEOUT
 */
err_t Sem_Lock(sem_t *sem, dtime_t timeout);

/**
 * @brief elease underlying resource
 *
 * @param sem lock to be released
 *
 * @return err_t status
 */
err_t Sem_Release(sem_t *sem);

/**
 * @brief Lock a set of semaphores, if lock is not possible at the moment the
 * all of locked semaphores will be released preventing deadlocks.
 *
 * @param sem_list null terminated list of semaphores
 * @param timeout operation timeout
 *
 * @return err_t error code
 */
err_t Sem_LockMultiple(sem_t **sem_list, dtime_t timeout);

/**
 * @brief release multiple semaphores
 *
 * @param sem_list null terminated list of semaphores
 *
 * @return err_t error code
 */
err_t Sem_ReleaseMultiple(sem_t **sem_list);

#endif /* _SYS_LOCK */
