/**
 * @file standby.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-03-04
 * 
 * @copyright Copyright (c) 2025
 */

#include "err.h"
#include "arch/arch.h"
#include "dev/gpio_signals.h"
#include "stm32f401/pwr.h"
#include "stm32f401/scb.h"

/* initialize support for the standby mode */
err_t StandBy_Init(void)
{
    return EOK;
}

/* enter standby mode */
void StandBy_Enter(void)
{
    /* this function shall put us into standby mode */
    while (1) {
        /* enable wakeup pin */
        PWR->CSR |= PWR_CSR_EWUP;
        /* power down deep sleep, clear wake-up flag */
        PWR->CR |= PWR_CR_PDDS | PWR_CR_CWUF;

        /* make sure that all writes took place */
        Arch_DSB();
        /* WUF is cleared after two clock cycles */
        Arch_NOP();
        Arch_NOP();

        /* prepare the process for deep sleep */
        SCB->SCR |= SCB_SCR_SLEEPDEEP;

        /* make sure that all writes took place */
        Arch_DSB();
        /* go to sleep */
        Arch_WFE();
        /* make sure that instuction is executed */
        Arch_ISB();
    }
}
