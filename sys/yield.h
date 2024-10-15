/**
 * @file yield.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-27
 * 
 * @brief Yield - context switcher
 */

#ifndef SYS_YIELD
#define SYS_YIELD

#include <stddef.h>

#include "err.h"
#include "sys/time.h"


/** @brief function type for task handler routine */
typedef void (* yield_hndl_t)(void *);

/** coroutine type  */
typedef struct yield_coro_t {
    /* coroutine handler */
    yield_hndl_t handler;
    /* coroutine argument */
    void *arg;
} yield_coro_t;


/** @brief Context switch service routine */
void Yield_PendSVHandler(void);

/**
 * @brief Starts the context switcher. Needs to be called before any context 
 * switch takes place
 * 
 * @return err_t error code
 */
err_t Yield_Init(void);

/**
 * @brief start the context switcher. Never returns
 */
void Yield_Start(void);

/**
 * @brief Create a task, but do not start it right away - it will be started 
 * after a call to Yield() is issued.
 * 
 * @param handler task handler routine
 * @param arg argument passed to that handler
 * @param stack stack for the task
 *
 * @return err_t error code when something is goes wrong or a positive number 
 * that is the task id 
 */
err_t Yield_Task(yield_hndl_t handler, void *arg, size_t stack_size);


/**
 * @brief Runs any given handler as a coroutine
 * 
 * @param handler function to be executed as a coroutine
 * @param arg function argument
 * @param flags modes of execution
 * 
 * @return err_t error code 
 */
err_t Yield_Run(void (*handler)(void *), void *arg);


/**
 * @brief wait for the task with givn id to be finished 
 * 
 * @param task_id id of the task to wait for
 * @return err_t 
 */
err_t Yield_Wait(int task_id, dtime_t timeout);


/**
 * @brief wait for all tasks to be finished
 * 
 * @param task_ids list of task ids (zero terminated)
 * @param timeout timeout value
 * 
 * @return err_t error code 
 */
err_t Yield_WaitAll(int task_ids[], dtime_t timeout);


/**
 * @brief execute coroutines in parallel  
 * 
 * @param coros zero terminated list of coroutines
 * @param task_ids placeholder for the task ids of all tasks that for 
 * all coroutines (must be the same length as 'coros')
 * 
 * @return err_t error code 
 */
err_t Yield_Parallel(yield_coro_t coros[], int task_ids[]);

/**
 * @brief Yields from current task, does a context switch and continues the 
 * execution of other tasks
 */
void Yield(void);

/**
 * @brief returns current task id
 * 
 * @return int task id 
 */
int Yield_GetTaskID(void);

/**
 * @brief shield from cancellation 
 * 
 * @param enable 1 - enable shield, 0 - disable shield
 */
void Yield_Shield(int enable);

/**
 * @brief Cancel any given task
 * 
 * @param task_id task to be cancelled
 * 
 * @return err_t EOK if cancellation was successful 
 */
err_t Yield_Cancel(int task_id);

/**
 * @brief cancel all tasks identified by their ids
 * 
 * @param task_ids 0-terminated task id array
 */
void Yield_CancelAll(int task_ids[]);

/**
 * @brief is current task cancelled
 * 
 * @return int 1 - task is cancelled, 0 - task is running normally
 */
int Yield_IsCancelled(void);


#endif /* SYS_YIELD */
