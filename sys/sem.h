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
#include "compiler.h"

/** semaphore lock typedef */
typedef enum sem { SEM_RELEASED, SEM_LOCKED } sem_t;


/** macro for doing an operation with a semaphore */
#define with_sem(sem)                                                                       \
    for (sem_t __once = Sem_Lock(sem, 0) + 1, * CLEANUP(with_sem_cleanup) __sem = (sem);    \
         __once > 0; __once = 0)


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


/* cleanup routine for the with_sem macro */
static inline void with_sem_cleanup(sem_t **sem) { Sem_Release(*sem); }
/* cleanup routine for the with_sem macro */
static inline void with_sems_cleanup(sem_t ***sems) { Sem_ReleaseMultiple(*sems); }

#endif /* _SYS_LOCK */
