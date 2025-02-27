/**
 * @file beep.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-27
 * 
 * @copyright Copyright (c) 2025
 */

#include "config.h"
#include "err.h"

#include "dev/gpio_signals.h"
#include "sys/critical.h"
#include "stm32f401/rcc.h"
#include "stm32f401/timer.h"
#include "sys/sleep.h"

#define DEBUG DLVL_INFO
#include "debug.h"

/* initialize beeper */
err_t Beep_Init(void)
{
    /* do we need to multiply the peripheral clock by 2? it is the case when
     * peripheral clock is prescaled by RCC */
    int mult = 1;

    /* enter critical section */
	Critical_Enter();

    /* clock is prescaled */
	if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1)
        mult = 2;

    /* enable tim3 */
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    /* 1us per pulse (multiply by two may be needed)*/
    TIM3->PSC = (APB1CLOCK_HZ * mult) / 1000000 - 1;
    /* setup for 4kHz freq (period = 250 us) */
    TIM3->ARR = 250 - 1;
    /* reload prescaler */
    TIM3->EGR = TIM_EGR_UG;

    TIM3->SMCR |= TIM_SMCR_MSM;
    /* setup the compare value */
    TIM3->CCR4 = TIM3->ARR / 2;
    /* setup pwm mode */
    TIM3->CCMR2 = TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1;

    /* enable timer */
    TIM3->CR1 = TIM_CR1_CEN;

    /* configure pin */
    GPIOSig_CfgAltFunction((gpio_signal_t)GPIO_SIGNAL_C9,
        GPIO_AF_TIM3_TIM4_TIM5);

    /* enter critical section */
	Critical_Exit();

    /* report status */
    return EOK;
}

/* do a beep */
err_t Beep_Beep(dtime_t duration_ms)
{
    /* sanitize the input */
    if (duration_ms < 0)
        return EARGVAL;

    /* enable timer output */
    TIM3->CCER |=  TIM_CCER_CC4E;
    /* allow beeper to beep */
    Sleep(duration_ms);
    /* disable timer output */
    TIM3->CCER &= ~TIM_CCER_CC4E;

    /* report status */
    return EOK;
}

/* do a beep */
err_t Beep_Set(int state)
{
    /* caller wants to set the beeper on? */
    if (state) {
        TIM3->CCER |=  TIM_CCER_CC4E;
    /* caller wants to set the beeper of? */
    } else {
        TIM3->CCER &= ~TIM_CCER_CC4E;
    }

    /* report status */
    return EOK;
}
