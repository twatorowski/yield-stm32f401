/**
 * @file yield.c
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-03-27
 * 
 * @brief Yield - context switcher
 */

#include <stdint.h>
#include <stddef.h>

#include "assert.h"
#include "compiler.h"
#include "config.h"
#include "err.h"
#include "linker.h"
#include "arch/arch.h"
#include "dev/watchdog.h"
#include "stm32f401/stm32f401.h"
#include "stm32f401/nvic.h"
#include "stm32f401/scb.h"
#include "sys/heap.h"
#include "sys/time.h"
#include "sys/yield.h"
#include "util/elems.h"

/* tasks stack frame - backwards since it it placed on stack, in basic mode: 
 * a.k.a no floating point registers */
typedef struct {
    /* exception return code */
    uint32_t exc_return;
    /* registers that we put manually on stack */
    uint32_t r11, r10, r9, r8, r7, r6, r5, r4;
    /* general purpose registers that are put by the interrupt */
    uint32_t r0, r1, r2, r3, r12;
    /* link register, program counter and status register */
    uint32_t lr, pc, xpsr;
} task_frame_basic_t;

/* tasks stack frame - backwards since it it placed on stack, in extended mode: 
 * a.k.a with floating point registers */
typedef struct {
    /* exception return code */
    uint32_t exc_return;
    /* 16 general purpose floating point registers */
    float s16_31[16];
    /* registers that we put manually on stack */
    uint32_t r11, r10, r9, r8, r7, r6, r5, r4;
    /* general purpose registers that are put by the interrupt */
    uint32_t r0, r1, r2, r3, r12;
    /* link register, program counter and status register */
    uint32_t lr, pc, xpsr;
    /* 16 general purpose floating point registers */
    float s00_15[16];
    /* floating point control register */
    uint32_t fpcsr;
} task_frame_ext_t;

/* task stack frame in two modes */
typedef union {
    /* basic mode - no fpu */
    task_frame_basic_t basic;
    /* extended mode - with fpu */
    task_frame_ext_t ext;
} task_frame_t;

/* task descriptor */
typedef struct task {
    /* stack pointer address */
    task_frame_t *sp;

    /* pointer to next and previous task control block */
    struct task *next, *prev;

    /* task state */
    enum task_state { TASK_PENDING, TASK_ACTIVE, TASK_DONE } state;
    /* flags  */
    enum task_flags { TASK_FLAGS_COROUTINE = 0x1 } flags;

    /* task handler */
    void (* handler) (void *);
    /* task handler argument */
    void *handler_arg;
    /* flag that indicates the handler execution is done */
    int handler_done;

    /* shielded from cancellation? marked for cancellation? */
    int shielded, cancelled;

    /* stack and stack size */
    void *stack;
    /* size of stack */
    size_t stack_size;

    /* task id */
    int id;
} task_t;

/* current task pointer */
static task_t *curr_task;
/* context switch counter */
static volatile uint32_t switch_cnt, task_cnt;
/* task id to be assigned to the next task that is created */
static int next_task_id = 1;

/* tasks that wrap coroutines */
static task_t *coroutines[SYS_CORO_MAX_NUM];
 

/* initiate context switch procedure */
static void Yield_CallScheduler(void)
{
    /* set pend sv, ensure that we've reached the switcher routine */
    SCB->ICSR |= SCB_ICSR_PENDSVSET;
}

/* task executor wrapper */
static void Yield_ExecuteTask(task_t *t)
{
    /* execute task handler */
    t->handler(t->handler_arg);
    /* change this flag to notify all awaiters */
    t->handler_done = 1;
    /* go through all tasks to notify */
    Yield_CallScheduler();

    /* mark the task as done */
    t->state = TASK_DONE;
    /* switch the context to do a cleanup */
    Yield_CallScheduler();
}

/* allocate memory for the stack on which the task will function */
static task_t * Yield_AllocateTask(size_t stack_size)
{
    /* size of the stack and the frame */
    size_t stack_and_frame_size = stack_size + sizeof(task_frame_t);

    /* allocate memory for the stack */
    void *stack = Heap_Malloc(stack_and_frame_size);
    /* unable to allocate the memory for stack */
    if (!stack)
       goto cleanup;

    /* this pointer will hold the task descriptor table entry */
    task_t *t = Heap_Malloc(sizeof(task_t));
    /* unable to allocate memory for the task itself */
    if (!t)
        goto cleanup;
    
    /* store the pointers within the task record */
    t->stack = stack; t->stack_size = stack_and_frame_size;
    /* return task pointer */
    return t;

    /* release the memory */
    cleanup: Heap_Free(stack); Heap_Free(t);
    /* report fail */
    return 0;
}

/* free up the memory that belongs to the task */
static void Yield_DeallocateTask(task_t *t)
{
    /* free up both: stack and task control bloxk */
    Heap_Free(t->stack); 
    Heap_Free(t);
}

/* fill in task control block information */
static void Yield_InitializeTask(task_t *t, void (*handler)(void *), void *arg, 
    enum task_flags flags)
{
    /* shorthands */
    void *stack = t->stack; size_t stack_size = t->stack_size;
    
    /* initial checks */
    assert(((uintptr_t)stack & 3) == 0, "stack must be word-aligned");
    assert((stack_size & 3) == 0, "stack size must be a multiple of 4");
    assert(stack_size >= sizeof(task_frame_t), "stack size is too small");

    /* set task id */
    t->id = next_task_id++;
    /* store execution handler and it's argument */
    t->handler = handler; t->handler_arg = arg; t->handler_done = 0;
    /* clear cancellation flag */
    t->cancelled = 0;

    /* word with lowest address shall carry the guard word */
    *(uint32_t *)stack = 0xdeadc0de;

    /* set stack pointer to the top of the stack - the size of the stack frame. 
     * this will allow the context switch routine to load the values from the 
     * stack */
    t->sp = (task_frame_t *)((uintptr_t)stack + stack_size);
    /* ensure that we are aligned to 8 byte boundary after the context switcher 
     * pops all the registers from the stack - this is what calling 
     * convention expects. Then, move the pointer back by the amount required to 
     * fit the basic stack frame */
    t->sp = (task_frame_t *)(((uintptr_t)t->sp & ~0x7) - 
        sizeof(task_frame_basic_t));

    /* default status register value: Thumb bit set */
    t->sp->basic.xpsr = 0x01000000;
    /* set the program counter to point to the task routine */
    t->sp->basic.pc = (uint32_t)Yield_ExecuteTask;
    /* debug value */
    t->sp->basic.lr = 0xdeadc0de;
    /* set the task pointer address within the r0 as the r0 is where the 1st 
     * function argument is kept by ARM calling convention */
    t->sp->basic.r0 = (uint32_t)t;
    /* exc return code indicates basic stack frame (no fpu) and psp as stack 
     * pointer - every new task starts that way, but things may evolve if user 
     * uses floating point arithmetic operations */
    t->sp->basic.exc_return = 0xFFFFFFFD;
    /* set task state to pending - scheduler will take it from here */
    t->state = TASK_PENDING;

    /* reset the flags  */
    t->flags = flags;

    /* 1st task ever? */
    if (!curr_task) {
        /* setup single task scenario */
        curr_task = t->prev = t->next = t;
    /* more tasks currently being enqued? */
    } else {
        /* put current task in place of 'tasks' pointer */
        t->prev = curr_task->prev;
        t->next = curr_task;
        /* update tasks */
        curr_task->prev->next = t;
        curr_task->prev = t;
    }

    /* bump up teh task counter */
    task_cnt++;
}

/* validate that tasks' stack was not corrupted */
static void Yield_CheckStack(void)
{
    /* these checks are valid only for tasks that have their own stack: i.e. 
     * subtasks of the main task */
    /* check for overflows */
    assert((uintptr_t)curr_task->sp > (uintptr_t)curr_task->stack, 
        "stack overflow");
    /* check the stack guard */
    assert(*(uint32_t *)curr_task->stack == 0xdeadc0de, 
        "stack guard corrupted");
}

/* select next task to be executed */
static void Yield_Schedule(void)
{
    /* bump up the counter */
    switch_cnt++;

    /* loop until next task is found */
    while (1) {
        /* task is completed? */
        if (curr_task->state == TASK_DONE) {
            /* advance the pointer */
            task_t *t = curr_task; curr_task = curr_task->next;
            /* unlink the task from the list */
            t->prev->next = t->next;
            t->next->prev = t->prev;      
            /* sanity check */
            assert((t->prev != t) && (t->next != t), "all tasks executed!");

            /* release the memory if the task is not coroutine */
            if (!(t->flags & TASK_FLAGS_COROUTINE))
                Yield_DeallocateTask(t);
            /* consume task */
            task_cnt--;
        /* active task? make it into pending */
        } else if (curr_task->state == TASK_ACTIVE) {
            /* go back to pending state */
            curr_task->state = TASK_PENDING;
            /* move to the next state */
            curr_task = curr_task->next;
        /* pending tasks go here */
        } else {
            /* mark as active */
            curr_task->state = TASK_ACTIVE;
            /* switch */
            break;
        }
    }
}

/* get task by task id number */
static task_t * Yield_GetTaskByID(int task_id)
{
    /* look wihin the tasks for a task with given id */
    for (task_t *t = curr_task; ;) {
        /* it found then return the task control block */
        if (t->id == task_id) return t;
        if ((t = t->next) == curr_task) return 0;
    }
}

/* context switch interrupt */
void NAKED OPTIMIZE ("Os") Yield_PendSVHandler(void)
{
    /* stack pointer holding register */
    register task_frame_t *sp;

    /* we are in the naked function, and as such the mcu has already built the 
     * stack frame that consists of xpsr, pc, lr, r12, r3, r2, r1, r0 */
    ASM volatile (

        /* load the stack pointer */
        "mrs r0, psp                    \n"
        "isb                            \n"

        /* store all other registers */
        "stmdb r0!, {r4-r11}            \n"

        /* second thing that is encoded in EXC RETURN's value is the presence of 
         * fpu registers stacking: if the 4th bit is set to zero then we need 
         * to stack the floating point registers */
        "tst lr, #0x00000010            \n"
        "it eq                          \n"
        "vstmdbeq r0!, {s16-s31}	    \n"

        /* let's store the RETURN value to have all the information needed for 
         * stack restoration process */
        "stmdb r0!, {lr}                \n"

        /* copy the stack pointer value */
        "mov %[sp], r0                  \n"
        /* write operands */
        : [sp] "=r" (sp)
    );

    /* store stack pointer of current task */
    curr_task->sp = sp;
    /* validate stack of task that yielded */
    Yield_CheckStack();
    /* kick the dog */
    Watchdog_Kick();
    /* select next task for the execution */
    Yield_Schedule();
    /* use it's stack pointer to restore it's stack frame */
    sp = curr_task->sp;
    
    /* load registers r4-r11 from next task frame */
    ASM volatile (

        /* read the EXC_RETURN code for the next task */
        "ldmia %[sp]!, {r0}             \n"

        /* test for the floating point context */
        "tst r0, #0x00000010            \n"
        "it eq                          \n"
        "vldmiaeq %[sp]!, {s16-s31}	    \n"

        /* read all the registers that we need to read manually */
        "ldmia %[sp]!, {r4-r11}         \n"
        
        /* restore the stack pointer */
        "msr psp, %[sp]                 \n"
        "isb                            \n"
        /* continue with next task by returning appropriate EXC_RETURN code */
        "bx r0                          \n"
        /* write operands */
        : [sp] "+r" (sp)
    );
}

/* start the yield context switcher */
err_t Yield_Init(void)
{
    /* set the context switcher priority to the lowest possible level */
    SCB_SETEXCPRI(STM32_EXC_PENDSV, INT_PRI_YIELD);

    /* return status */
    return EOK;
}

/* start the context switcher */
void Yield_Start(void)
{
    /* sanity check */
    assert(curr_task, "no tasks are due for execution");

    /* setup initial stack value. since we are making a normal call to handler 
     * we do not need to create stack frame */
    curr_task->sp = (task_frame_t *)((uintptr_t)curr_task->stack + 
        curr_task->stack_size);
    curr_task->sp = (task_frame_t *)((uintptr_t)curr_task->sp & ~0x7);
    curr_task->state = TASK_ACTIVE;

    /* setup stack pointer */
    Arch_WritePSP(curr_task->sp);
    Arch_ISB();
    /* start using the program stack pointer */
    Arch_WriteCONTROL(0x02);
    /* call the handler */
    curr_task->handler(curr_task->handler_arg);

    /* endless loop */
    while (1)
        Yield();
}

/* prepare task for the execution */
err_t Yield_Task(void (*handler)(void *), void *arg, size_t stack_size)
{    
    /* allocate memory for the task */
    task_t *t = Yield_AllocateTask(stack_size);
    /* no memory left  */
    if (!t)
        return EFATAL;

    /* fill in the task control block information */
    Yield_InitializeTask(t, handler, arg, 0);
    /* return task id */
    return t->id;
}

/* run handler as a coroutine */
err_t Yield_Run(void (*handler)(void *), void *arg)
{
    /* coroutine control block */
    task_t **coro; int found = 0, test= 0;

    /* wait for a free slot in coroutine pool */
    for (; !found; Yield()) {
        /* go through all coroutine control blocks and find one that can be 
         * used for starting the coroutine  */
        for (coro = coroutines; coro != coroutines + elems(coroutines); coro++) {
            /* this means either free slot or a slot occupied by the task that 
             * is already completed */
            if (!(*coro) || (*coro)->state == TASK_DONE) {
                found = 1; break;
            }
        }
        if (!found)
            test++;
    }

    /* empty slot, we need to allocate memory */
    if (!*coro && !(*coro = Yield_AllocateTask(SYS_CORO_STACK_SIZE)))
        return EFATAL;

    /* prepare the task for execution */
    Yield_InitializeTask(*coro, handler, arg, TASK_FLAGS_COROUTINE);
    /* return the coroutine task id if we are not waiting for the task */
    return (*coro)->id;
}

/* wait for the task to be finished */
err_t Yield_Wait(int task_id, dtime_t timeout)
{
    /* look for the task with this id */
    task_t *t = Yield_GetTaskByID(task_id); time_t ts = time(0);
    /* task must be already completed */
    if (!t)
        return EOK;

    /* task is ongoing, wait until it is finished */
    for (; !t->handler_done; Yield())
        if (timeout && dtime_now(ts) > timeout)
            return ETIMEOUT;
        
    /* waiting is done */
    return EOK;
}

/* wait for all tasks to be finished */
err_t Yield_WaitAll(int task_ids[], dtime_t timeout)
{
    /* current time for the sake of timeout computation */
    time_t ts = time(0); err_t ec;
    /* wait for the corotuines to be finished */
    for (int i = 0; task_ids[i]; i++) {
        if (task_ids[i]) {
            if ((ec = Yield_Wait(task_ids[i], 
                timeout ? timeout - dtime_now(ts): 0)) != EOK)
                return ec;
        }
    }

    /* everything has been completed */
    return EOK;
}

/* execute coroutines in parallel  */
err_t Yield_Parallel(yield_coro_t coros[], int task_ids[])
{
    /* coroutine pointer  */
    yield_coro_t *coro;
    /* run all of the coroutines */
    for (coro = coros; coro->handler; coro++) {
        /* try to run the coroutine */
        err_t ec = Yield_Run(coro->handler, coro->arg);
        /* error during task running */
        if (ec < EOK)
            return ec;
        /* store the task id */
        if (task_ids)
            task_ids[coro - coros] = ec; 
    }
    
    /* zero terminate */
    task_ids[coro - coros] = 0;
    /* return status */
    return coro - coros;
}



/* yield from current task */
void Yield(void)
{
    /* call the scheduler */
    Yield_CallScheduler();
}

/* get current task id */
int Yield_GetTaskID(void)
{
    /* return current task id */
    return curr_task->id;
}

/* shield from cancellation */
void Yield_Shield(int enable)
{
    /* store the shield flag value */
    curr_task->shielded = !!enable;
}

/* cancel a task */
err_t Yield_Cancel(int task_id)
{
    /* get the task by it's id */
    task_t *t = Yield_GetTaskByID(task_id);
    /* mark as cancelled */
    if (t) {
        t->cancelled = 1; return EOK;
    }

    /* cannot cancel a non existing task */
    return EFATAL;
}

/* cancel all tasks identified by their ids */
void Yield_CancelAll(int task_ids[])
{
    /* do the cancellation on all tasks from the array */
    for (int i = 0; task_ids[i]; i++)
        Yield_Cancel(task_ids[i]);
}

/* is the current task cancelled? */
int Yield_IsCancelled(void)
{
    /* return the state of is cancelled flag */
    return !curr_task->shielded && curr_task->cancelled;
}