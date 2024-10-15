/**
 * @file systime.c
 * 
 * @date 2021-01-26
 * twatorowski (tomasz.watorowski@gmail.com)
 * 
 * @brief System timer
 */

#include "config.h"
#include "err.h"
#include "sys/critical.h"
#include "stm32f401/rcc.h"
#include "stm32f401/timer.h"

/* reset time base */
int SysTime_Init(void)
{
	/* do we need to multiply the peripheral clock by 2? it is the case when 
	 * peripheral clock is prescaled by RCC */
	int mult = 1;

	/* enter critical section */
	Critical_Enter();

	/* clock is prescaled */
	if ((RCC->CFGR & RCC_CFGR_PPRE1) != RCC_CFGR_PPRE1_DIV1)
		mult = 2;
	
	/* enable tim2, tim6 clock */
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM2EN;
    /* wait for the timer to be started */
    while ((RCC->APB1ENR & (RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM2EN)) != 
		(RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM2EN));


	/* 1us per pulse (multiply by two may be needed)*/
    TIM3->PSC = (APB1CLOCK_HZ * mult) / 1000000 - 1;
    /* maximal automatic reload value */
    TIM3->ARR = 0xffff;
    /* reload prescaler */
    TIM3->EGR = TIM_EGR_UG;
    /* enable timer */
    TIM3->CR1 = TIM_CR1_CEN;

	/* set prescaler to obtain 100us pulse */
	TIM2->PSC = ((APB1CLOCK_HZ * mult) / 10000) - 1;
	/* set autoreload value */
	TIM2->ARR = 0xffffffff;
	/* reset value */
	TIM2->CNT = 0;
	/* set update event */
	TIM2->EGR = TIM_EGR_UG;
	/* enable timer */
	TIM2->CR1 = TIM_CR1_CEN;

	/* exit critical section */
	Critical_Exit();

	/* not much could go wrong here */
	return EOK;
}

/* get time */
uint32_t SysTime_GetTime(void)
{
    /* return the timer value in 100us ticks */
    return TIM2->CNT;
}

/* get current microsecond timer value */
uint16_t SysTime_GetUs(void)
{
	return TIM3->CNT;
}