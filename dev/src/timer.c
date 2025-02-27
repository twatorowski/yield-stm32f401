/**
 * @file timer.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-27
 * 
 * @copyright Copyright (c) 2025
 */

#include "err.h"


/* initialize timer */
err_t Timer_Init(void)
{
    /* enter critical section */
    Critical_Enter();

    /* enable timers clock */
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM2EN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM2EN;

    /* exit critical section */
    Critical_Exit();
}