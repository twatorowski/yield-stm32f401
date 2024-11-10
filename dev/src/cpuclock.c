/**
 * @file cpuclock.c
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-09-02
 * 
 * @brief Sets the cpu clock
 */

#include "config.h"
#include "err.h"
#include "stm32f401/rcc.h"
#include "stm32f401/flash.h"
#include "stm32f401/pwr.h"
#include "stm32f401/scb.h"
#include "sys/critical.h"
#include "util/msblsb.h"


/** sanitize the clock settings  */
#if (APB1CLOCK_HZ != 42000000) || (APB2CLOCK_HZ != 84000000)
    #error "please update the speed enum to match new clock settings"
#endif

/* prepare cpu clock for operation */
err_t CpuClock_Init(void)
{
    /* enter critical section */
    Critical_Enter();

    /* enable power to system configurator */
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /* enable crystal oscillator 25MHz */
    RCC->CR |= RCC_CR_HSEON;
    /* wait till its enabled */
    while (!(RCC->CR & RCC_CR_HSERDY));

    /* configure f_out = 25MHz (crystal) / 25 * 336 / 4 = 84MHz
     * additionaly configure pll to output 48MHz for the USB */
    RCC->PLLCFGR = RCC_PLLCFGR_PLLSRC | 336 << LSB(RCC_PLLCFGR_PLLN) | 
        1 << LSB(RCC_PLLCFGR_PLLP) | 25 << LSB(RCC_PLLCFGR_PLLM) |
        7 << LSB(RCC_PLLCFGR_PLLQ);

    /* start the pll */
    RCC->CR |= RCC_CR_PLLON;
    /* wait for the pll to become ready */
    while (!(RCC->CR & RCC_CR_PLLRDY));


    /* configure divisions for the busses so that we do not exceed the 
     * maximal speeds */
    RCC->CFGR = RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE2_DIV1;

    /* program the number of wait states */
    FLASH->ACR = FLASH_ACR_LATENCY_2WS;
    /* wait for the option to be applied */
    while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_2WS);

    /* and finally switch the clock */
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;
    /* wait for the switch to occur */
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);


    /* exit critical section */
    Critical_Exit();
    /* report status */
    return EOK;
}