/**
 * @file time.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2024-10-14
 * 
 * @copyright Copyright (c) 2024
 */

#include "assert.h"
#include "config.h"
#include "err.h"

#include "arch/arch.h"
#include "stm32f401/scb.h"
#include "stm32f401/systick.h"


/* reload register value */
#define RELOAD                              ((AHBCLOCK_HZ / 8) - 1)
/* number of systick overflows, should be incremented by 1000 per overflow */
static volatile uint32_t ticks;

/* systick overflow exception handler */
void Time_TickHander(void)
{
    /* bump up the ticks counter */
    ticks += 1000;
}

/* intialize system timer circuitry */
err_t Time_Init(void)
{
    /* set the context switcher priority to the lowest possible level */
    SCB_SETEXCPRI(STM32_EXC_SYSTICK, INT_PRI_SYSTICK);

    /* sanity checks */
    assert(RELOAD > 1e6, 
        "reload value is low, consider speeding up the systick");
    assert(RELOAD <= SYSTICK_LOAD_RELOAD, 
        "reload value too high - slow down the systick timer");

    /* setup the reload value */
    SYSTICK->LOAD = RELOAD;
    /* start the timer and enable interrupt generation */
    SYSTICK->CTRL |= SYSTICK_CTRL_ENABLE | SYSTICK_CTRL_TICKINT;

    /* initialize  */
    return EOK;
}

/* return the time in ms */
uint32_t OPTIMIZE("O3") Time_GetTime(void)
{
    uint32_t ms;
    /* loop as long as we do not have a stable read from 'ticks' and 
     * systick->val */
    while (1) {
        /* this allows to check whether interrupt has occured between the 
         * calls of ldrex and strex */
        Arch_LDREX(&ms);
        /* ensure that we read the fresh value of 'ticks' */
        Arch_DSB();
        /* if this equals 0 then no intterupt has occured  since ldrex */
        if (Arch_STREX(&ms, ticks + (RELOAD - SYSTICK->VAL) / (RELOAD / 1000)) == 0)
            return ms;
    }
}

/* get micoseconds value */
uint32_t OPTIMIZE("O3") Time_GetUS(void)
{
    /* get the value and leave the "microseconds" part */
    uint32_t us = (RELOAD - SYSTICK->VAL) % (RELOAD / 100);
    /* scale to microseconds */
    return us * 10000 / (RELOAD / 100);
}

/* simple delay function */
void OPTIMIZE("O3") Time_DelayUS(uint32_t us)
{
    /* microsecond timestamps */
    uint32_t curr_us, prev_us = Time_GetUS();

    /* loop until timing is met */
    for (uint32_t elapsed = 0; elapsed < us; ) {
        /* get current timestamp */
        curr_us = Time_GetUS();
        /* get the difference between timestamps */
        uint32_t diff = curr_us - prev_us;
        /* us counter overflow? */
        if (diff > 10000)
            diff = 10000 + diff;
        /* update the elapsed counter */
        elapsed += diff; prev_us = curr_us;
    }
}