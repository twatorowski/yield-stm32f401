/**
 * @file fpu.c
 *
 * @date 29.06.2019
 * @author twatorowski (tomasz.watorowski@gmail.com)
 *
 * @brief Floating Point Unit
 */

#include "err.h"
#include "arch/arch.h"
#include "stm32f401/scb.h"

/* initialize fpu */
err_t FPU_Init(void)
{
    /* enable co-processor access */
    SCB->CPACR |= 0xf << 20;

    /* synchronize cpu */
    Arch_DSB();
    Arch_ISB();

    /* report status */
    return EOK;
}
