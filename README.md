# Yield Operating System for STM32F401 (Blackpill)

Copyright: 2024 Tomasz Watorowski, License: MIT

`Yield` is a simple operating system built around the idea of cooperative
multi-tasking (as opposed to preemptive multi-tasking) which is perfect 
for 99.99999% of the jobs I am doing. Other stuff (more time critical) can be addressed using interrupt based approach.

This OS was born out of pure fascination for asynchronous processing model that
is present in many programming languages like python and javascript.

My goal was to mimic what is provided by python's asyncio model. My other goal 
was to write EVERYTHING from scratch because this is how you learn. No pain,
no gain.

I've ported this OS to STM32F401 Blackpill board because I'm a big fan of STM32
MCUs and Blackpill boards are super cheap and accessible. As far as I know there
should be no problem with running this project on STM32F411 Blackpill boards
without any modification.


## What's inside?

This OS comes with couple of things: 

* Cooperative round-robin scheduler, it's very simple (no priorities) and 
ensures that next time your task is being executed all other tasks had been 
run as well (that makes synchronization super-easy). In order to pass control
to other tasks you just simply call `Yield()` function.

* You can start execution of tasks in parallel using `Yield_Parallel()` (or 
`Yield_Run()` in case of a single task), wait for the results using 
`Yield_Wait()`, cancel the tasks `Yield_Cancel()`, shield the task from 
cancellation using `Yield_Shield()`.

* System Timer (based around arm's SYSTICK). Provides millisecond and 
microsecond (it mcu clock allows) resolution. Calling `Sleep(100)` will give 
the control to other tasks for 100ms. Calling `Time_DelayUS(30)` will stall
the execution (control will not be passed to other tasks) for 30 microseconds,
which is useful if you are doing some timed bitbanging.

* Dynamic memory with functions like `Heap_Malloc()` and `Heap_Free()`.

* Semaphores `sem_t` with options to lock on multiple of them without the risk
of deadlocking (`Sem_LockMultiple()` and `Sem_ReleaseMultiple()`)

* Queues for passing data between tasks (see `queue.h`)

* Utilities like simplified versions `stdio.h`, `string.h`, etc...

* Drivers for most popular peripherals like `gpio`, `usart`, `spi`, `i2c`

* printf-like debug routine connected to usart driver `dprintf("Hello, World %d\n", 123);`


## How to build

I don't like complex build systems and I don't use them in my personal projects.
This entire project can be build with `arm-none-eabi-*` toolchain + `make`. If 
you have the toolchain installed on your machine then the only thing you type in
is:
```
make
``` 

If you don't have the the toolchain and you don't want to contaminate your PC 
with the toolchain you can build the project using docker container that I've
prepared for such projects:
```
make docker_build
```

If you are using `VScode` as your IDE I've also included the tasks.json, 
settings.json and launch.json. Then you can build using the build menu 
available after hitting `CTRL+SHIFT+B`

## How To Use

`main.c` is the main (duh) file of the project. I've included couple of 
examples that show how to use the OS. 

### Creating a Task, sending data over UART

TLDR: Use `Yield_Task()` to create infinite loop tasks.

Creating tasks in such a way involves dynamic allocation of memory for task's 
stack, so it's for the best not to create short-lived tasks in that way as it 
may lead to constant allocation and deallocation of memory which, in turn will 
cause memory fragmentation. 


```c

/* an example of an endless task */
static void SendDataOverUart(void *arg)
{
    /* extract the argument  */
    const char *msg = arg;
    /* at the end of every iteration we give back the control to other tasks 
     * using 'Yield()' function */
    for (;; Yield()) {
        /* ensure exclusive access using this semaphore, it will get released 
         * when you exit this block. It's just a convenience notation but 
         * remember that you cannot 'return' or 'break' in this block, otherwise 
         * the semaphore will remain locked! */
        with_sem (&usart1.tx_sem) {
            USART_Send(&usart1, msg, strlen(msg), 0);
        }

        /* sleep between usart sends to avoid DDOsing your terminal :) */
        Sleep(5000);

        /* this is another way of achieving the same thing as above, but more 
         * explitit */
        Sem_Lock(&usart1.tx_sem, 0); {
            USART_Send(&usart1, msg, strlen(msg), 0);
        } Sem_Release(&usart1.tx_sem);

        /* sleep between usart sends to avoid DDOsing your terminal :) */
        Sleep(5000);
    }
}

/* EXAMPLE 1: Start an infinite task */
Yield_Task(SendDataOverUart, "Hello, World\n", 256);
```

### Starting a short-lived task.

TLDR: Use `Yield_Run()` to create a short lived task. Do not create infinite 
tasks in such way.

Short lived tasks can be  run within task pool. Task pool tasks have predefined 
task stack size (it's configurable from `config.h` [`SYS_CORO_STACK_SIZE`, `SYS_CORO_MAX_NUM`]). Task pool has predefined
size, that's why it would be unwise to run a task that is infinite since
it would occupy one pool element forever.

If there is no space on the pool then `Yield_Run()` will wait for space to 
become available.

```c
/* an example of a coroutine (async task that will finish) */
static void BlinkOnce(void *arg)
{
    /* blink the blue led once */
    Led_SetState(0, LED_BLU); Sleep(500);
    Led_SetState(1, LED_BLU); Sleep(500);
}
/* EXAMPLE 2: run a coroutine in parallel and wait for it to end */
/* run a routine on a task pool. this is cheaper since it does not 
    * allocate memory/deallocate memory each time a routine is being run, 
    * but the stack size for such tasks, is predefined (see config.h). 
    * Since the pool is limited in size one should not run infinte tasks. 
    * Routine will be started at the next iteration of the scheduler. Task id 
    * is returned, to allow for waiting for the routine to finish */
int blink_id = Yield_Run(BlinkOnce, 0);
/* wait for the routine to finish */
Yield_Wait(blink_id, 0);
```

### Starting couple of short lived tasks with different parameters

TLDR: For that we use `Yield_Parallel()` that takes a 0-terminated array
of handler-argument pair.

In this example we also show how to implement cancelation which is cooperative 
just like tasking i.e: no task will get stopped brutally, cancelation causes
a flag to be set and then it is users responsibility to cancel task gracefully.

```c
/* task used to demonstrate parallel executution */
static void SleepShoutAndDie(void *arg)
{
    /* extract the number of milliseconds */
    int ms = *((int*)arg);
    /* timestamp before we went to sleep */
    time_t start_of_sleep_timestamp = time(0);

    /* for the first 150ms we shield the task from cancellation */
    if (ms >= 150) {
        /* sleep will not return ECANCEL even if the task is cancelled 
         * thanks to shielding */
        Yield_Shield(1); Sleep(150); Yield_Shield(1);
        ms -= 150;
    }

    /* go to sleep, check if cancellation happened during sleep */
    if (Sleep(ms) == ECANCEL) {
        /* complain */
        dprintf("I slept only for %d out of %d ms!\n", 
            dtime_now(start_of_sleep_timestamp), ms);
        /* prevent further execution */
        return;
    }

    /* use printf like debug routine to communicate how much did we sleep for */
    dprintf("I've slept for the whole %d ms\n", ms);
}

/* EXAMPLE 3: run multiple coroutines in parallel, try to cancel them, 
    * wait for them to finish execution */
/* this will contain the ids of all tasks run in parallel and 
    * zero termination */
int task_ids[5];
/* an example of parallel execution */
Yield_Parallel((yield_coro_t[]) {
    /* start four coroutines with different arguments */
    { .handler = SleepShoutAndDie, .arg = &(int []) { 100 } },
    { .handler = SleepShoutAndDie, .arg = &(int []) { 200 } },
    { .handler = SleepShoutAndDie, .arg = &(int []) { 300 } },
    { .handler = SleepShoutAndDie, .arg = &(int []) { 400 } },
    /* array needs to be zero terminated */
    { 0 },
/* this argument is optional, if you don't want to wait for tasks to finish 
    * then set it to null */
}, task_ids);

/* sleep for some time */
Sleep(120);
/* cancel all tasks (even though some of them are already done) */
Yield_CancelAll(task_ids);

/* wait for tasks to finish */
Yield_WaitAll(task_ids, 0);
```

### Queues for communication between tasks

TLDR: Use queues to pass messages between tasks. See `queue.h` for set of
functions available. Queue memory is dynamically allocated at it's creation.

```c

/* task for producing data and putting it into the queue */
static void QueueProducer(void *arg)
{
    /* extract the pointer to the queue from the argument */
    queue_t *q = arg;
    /* counter which value will be stored within the queue */
    int cnt = 0;

    for (;; Sleep(100)) {
        /* try to put the counter value into the queue , use 100ms of timeout */
        size_t size = Queue_PutWait(q, &cnt, 1, 500);
        /* no luck, queue is full */
        if (size == 0) {
            dprintf("timeout occured during Queue_PutWait\n", 0);
        /* value got stored */
        } else {
            dprintf("Value %d was stored in queue\n", cnt);
            /* increment the value for the next iteration */
            cnt++;
        }
    }
}

/* task for consuming data from the queue */
static void QueueConsumer(void *arg)
{
    /* extract the pointer to the queue from the argument */
    queue_t *q = arg;
    /* counter which value we want to read from the queue */
    int cnt;

    /* check if there is anything in the queue once per 2 seconds */
    for (;; Yield()) {
        /* get the data from the queue */
        size_t size = Queue_GetWait(q, &cnt, 1, 10);
        if (size) {
            dprintf("Extracted value %d from the queue\n", cnt);
        /* there is nothing in the queue, let's go to sleep */
        } else {
            dprintf("Queue is empty, going to sleep for a little bit\n", 0);
            Sleep(5000);
        }
    }
}

/* EXAMPLE 4: queue producer, queue consumer */
/* create a queue with the capacity for four 'int' like elements */
queue_t *q = Queue_Create(sizeof(int), 4);
/* start the indefinite tasks that produce and consume data to/from 
 * the queue */
Yield_Task(QueueProducer, q, 512);
Yield_Task(QueueConsumer, q, 512);
```

### Locking on multiple resources

TLDR: Use semaphores (`sem.h`) for ensuring exclusive access.

```c
static void ComplexFightOverLED(void *arg)
{
    /* delay used to simulate the randomness */
    int delay = *(int*)arg;
    /* create two semaphores that are shared between all tasks that 
     * run this routine */
    static sem_t sem_a = SEM_RELEASED, sem_b = SEM_RELEASED;

    /* lock */
    for (;; Yield()) {
        /* lock on both semaphores using convenience macro 'with_sems' */
        with_sems (((sem_t *[]){ &sem_a, &sem_b, 0 })) {
            /* blink led */
            Led_SetState(1, LED_BLU); Sleep(10 * delay);
            Led_SetState(0, LED_BLU); Sleep(10 * delay);
        }

        dprintf("Blinks were done by function with delay %d\n", delay);
    }
}

/* EXAMPLE 5 locking multiple semaphores */
Yield_Task(ComplexFightOverLED, &(int []) { 17 }, 512);
Yield_Task(ComplexFightOverLED, &(int []) { 19 }, 512);
```