/**
 * @file parallel.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @brief Support for parallel execution
 * @date 2024-10-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "sys/yield.h"


typedef struct {
    void (*handler)(void *);
    void *arg;
    int stack_size;
} task_list_t;

static void ExampleTask(void *arg)
{
    int *value = arg;
    Sleep(100);
    *value += 1;
}

/* execute the tasks in parallel */
err_t Parallel(task_list_t *tasks)
{
    for (task_list_t *t = tasks; *t->handler; t++)
        Yield_Task(t->handler, t->arg, t->stack_size);
    
}

void Test()
{
    

}