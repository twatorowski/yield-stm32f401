/**
 * @file watchdog.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-03-02
 * 
 * @copyright Copyright (c) 2025
 */

#include "compiler.h"
#include "defhndl.h"
#include "sys/time.h"
#include "stm32f401/rcc.h"
#include "stm32f401/nvic.h"
#include "stm32f401/wwdg.h"

/* last time we kicked the dog */
static uint32_t kick_head, kick_tail;

/* watchdog interrupt */
void Watchdog_WWDGIsr(void)
{
    /* we need to kick the dog at least once every 5 seconds */
    if (++kick_head - kick_tail < 100) {
        /* still within limits, kick the dog */
        WWDG->CR = WWDG_CR_T, WWDG->SR = ~WWDG_SR_EWIF;
    /* limits exceeded */
    } else {
        /* jump to default handler */
        ASM volatile("b DefHndl_DefaultHandler\n");
    }
}

/* initialize watchdog */
err_t Watchdog_Init(void)
{
	/* enable watchdog */
	RCC->APB1ENR |= RCC_APB1ENR_WWDGEN;

	/* configure for max interval */
	WWDG->CR = WWDG_CR_WDGA | WWDG_CR_T;
	/* set up prescaler and interrupt */
	WWDG->CFR = WWDG_CFR_EWI | WWDG_CFR_WDGTB | WWDG_CFR_W;

	/* set priority */
	NVIC_SETINTPRI(STM32_INT_WWDG, 0x00);
	/* enable interrupt */
	NVIC_ENABLEINT(STM32_INT_WWDG);


	/* what could possibly go wrong here */
	return EOK;
}

/* kick the dog! */
void Watchdog_Kick(void)
{
	/* reset counter */
	WWDG->CR = WWDG_CR_T;
	/* kick the counter */
	kick_tail = kick_head;
}