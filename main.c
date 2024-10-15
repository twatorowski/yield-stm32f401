/**
 * @file main.c
 *
 * @date 23.06.2019
 * @author twatorowski (tw@mightydevices.com)
 *
 * @brief main application file
 */

#include "compiler.h"
#include "vectors.h"

#include "dev/analog.h"
#include "dev/cpuclock.h"
#include "dev/dma.h"
#include "dev/fpu.h"
#include "dev/gpio.h"
#include "dev/led.h"
#include "dev/spi_dev.h"
#include "dev/spi.h"
#include "dev/swi2c_dev.h"
#include "dev/swi2c.h"
#include "dev/usart_dev.h"
#include "dev/usart.h"
#include "sys/heap.h"
#include "sys/sem.h"
#include "sys/sleep.h"
#include "sys/queue.h"
#include "sys/yield.h"
#include "util/string.h"


#define DEBUG
#include "debug.h"


/* an example of a coroutine (async task that will finish) */
static void BlinkOnce(void *arg)
{
    /* blink the blue led once */
    Led_SetState(0, LED_BLU); Sleep(500);
    Led_SetState(1, LED_BLU); Sleep(500);
}

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

/* program main function, must return int so that gcc does not complain in
 * pedantic mode (-Wmain) */
void Main(void *arg);

/* program init function, called before main (with interrupts disabled) */
void Init(void)
{
    /* initialize dynamic memory */
    Heap_Init();
    /* initialize system timer */
    Time_Init();
    /* start the context switcher */
    Yield_Init();

    /* create the entry task */
    Yield_Task(Main, 0, 2048);
    /* this shall initialize the scheduler */
    Yield_Start();
}


/* program main function */
void Main(void *arg)
{
    /* start the fpu */
    FPU_Init();
    /* configure the system clock */
    CpuClock_Init();

    /* initialize gpio */
    GPIO_Init();
    /* initialize dma controller */
    DMA_Init();
    /* initialize adc */
    Analog_Init();

    /* initialize usart driver */
    USART_Init();
    /* initialize usart devices */
    USARTDev_Init();

    /* initialize leds */
    Led_Init();
    /* drive the led */
    Led_SetState(1, LED_BLU);


    /* print a welcome message */
    dprintf("Welcome to Yield OS\n", 0);

    /* EXAMPLE 1: Start an infinite task */
    /* create a never-ending task that will run in background. Task will be 
     * started on the next iteration of OS's scheduler */
    Yield_Task(SendDataOverUart, "Hello, World\n", 256);


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


    /* EXAMPLE 4: queue producer, queue consumer */
    /* create a queue with the capacity for four 'int' like elements */
    queue_t *q = Queue_Create(sizeof(int), 4);
    /* start the indefinite tasks that produce and consume data to/from 
     * the queue */
    Yield_Task(QueueProducer, q, 512);
    Yield_Task(QueueConsumer, q, 512);


    /* EXAMPLE 5 locking multiple semaphores */
    Yield_Task(ComplexFightOverLED, &(int []) { 17 }, 512);
    Yield_Task(ComplexFightOverLED, &(int []) { 19 }, 512);



    /* infinite loop */
    for (;; Yield());
}
