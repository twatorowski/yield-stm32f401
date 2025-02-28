/**
 * @file valve.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-28
 * 
 * @copyright Copyright (c) 2025
 */

#include "err.h"

#include "dev/gpio_signals.h"
#include "dev/valve.h"

/* pin definitions */
#define GPIO_VALVE_EN                   (gpio_signal_t)GPIO_SIGNAL_B5

/* initialize valve support */
err_t Valve_Init(void)
{
    /* setup the output */
    GPIOSig_CfgOutput(GPIO_VALVE_EN, GPIO_OTYPE_PP, 0);
    /* return status */
    return EOK;
}

/* enable/disable the valve */
err_t Valve_Enable(int enable)
{
    /* drive the pin */
    GPIOSig_Set(GPIO_VALVE_EN, enable ? 1 : 0);
    /* return the status */
    return EOK;
}