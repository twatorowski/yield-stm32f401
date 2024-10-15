/**
 * @file dma.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-07-17
 * 
 * @brief DMA driver
 */

#include <stdint.h>
#include <stddef.h>

#include "compiler.h"
#include "err.h"
#include "stm32f401/rcc.h"
#include "util/bit.h"
#include "util/msblsb.h"
#include "sys/critical.h"


/* initialize dmas */
err_t DMA_Init(void)
{
    /* enter critical section */
    Critical_Enter();
    /* enable dma */
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMA2EN;
    /* exit critical section */
    Critical_Exit();

    /* report status */
    return EOK;
}
