/**
 * @file reset.c
 * 
 * @date 2019-11-13
 * twatorowski (tomasz.watorowski@gmail.com)
 * 
 * @brief Routine to reset the MCU
 */

#include "compiler.h"
#include "reset.h"
#include "stm32f401/scb.h"
#include "stm32f401/rcc.h"
#include "stm32f401/pwr.h"

/* rcc csr value */
static reset_src_t reset_src;

/* initialize reset support */
err_t Reset_Init(void)
{
    /* store the value of the reset rcc */
    uint32_t csr;

    /* enable power controller */
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;

    /* read the contents of the register */
    csr = RCC->CSR;
    /* clear the reset cause */
    RCC->CSR |= RCC_CSR_RMVF;

    /* translate the causes of the reset */
    if (csr & RCC_CSR_LPWRRSTF) reset_src |= RESET_SRC_LP;
    if (csr & RCC_CSR_WWDGRSTF) reset_src |= RESET_SRC_WWDG;
    if (csr & RCC_CSR_IWDGRSTF) reset_src |= RESET_SRC_IWDG;
    if (csr & RCC_CSR_SFTRSTF) reset_src |= RESET_SRC_SW;
    if (csr & RCC_CSR_PORRSTF) reset_src |= RESET_SRC_POR;
    if (csr & RCC_CSR_PINRSTF) reset_src |= RESET_SRC_PIN;
    if (csr & RCC_CSR_BORRSTF) reset_src |= RESET_SRC_BOR;

    /* read the contents of the register */
    csr = PWR->CSR;
    /* reset the flag */
    PWR->CR |= PWR_CR_CSBF;

    /* were we in standby mode? */
    if (csr & PWR_CSR_SBF) reset_src |= RESET_SRC_STANDBY;

    /* return status */
    return EOK;
}

/* get reset cause */
reset_src_t Reset_GetLastResetSource(void)
{
    /* return the value of the reset source */
    return reset_src;
}

/* resets the mcu */
void NORETURN Reset_ResetMCU(void)
{
    /* perform the reset using the scb */
    SCB->AIRCR = SCB_AIRCR_SYSRESETREQ | SCB_AIRCR_VECTKEY_WR;
    /* this is so that the gcc does not complain */
    while (1);
}
