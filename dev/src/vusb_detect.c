/**
 * @file vusb_detect.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-27
 * 
 * @copyright Copyright (c) 2025
 */

#include "err.h"
#include "dev/gpio_signals.h"

/* vusb detect pin */
#define GPIO_VUSB                           (gpio_signal_t)GPIO_SIGNAL_C4

/* initialize vusb detector */
err_t VUSBDet_Init(void)
{
    /* configure this pin as an input */
    GPIOSig_CfgInput(GPIO_VUSB);

    /* return status */
    return EOK;
}

/* is the vusb connected */
int VUSBDet_IsConnected(void)
{
    /* return status */
    return GPIOSig_Get(GPIO_VUSB);
}