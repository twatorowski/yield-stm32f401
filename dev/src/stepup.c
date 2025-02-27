/**
 * @file stepup.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-27
 * 
 * @copyright Copyright (c) 2025
 */

#include "err.h"
#include "dev/gpio_signals.h"
#include "sys/sleep.h"

/* converter pin */
#define PIN                             (gpio_signal_t)GPIO_SIGNAL_B4

/* is the dc/dc enabled? */
static int enabled;

/* initialize step up converter control */
err_t StepUp_Init(void)
{
    /* prepare the pin */
    return GPIOSig_CfgOutput(PIN, GPIO_OTYPE_OD, 1);
}

/* enable/disable step up converter */
err_t StepUp_Enable(int enable)
{
    /* drive the control pin */
    GPIOSig_Set(PIN, enable ? 0 : 1);
    /* small delay is applicable after enabling */
    if (!enabled && enable)
        Sleep(10);
    /* store the state */
    enabled = enable;

    /* not much can fail here */
    return EOK;
}