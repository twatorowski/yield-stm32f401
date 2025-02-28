/**
 * @file stepup.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-27
 * 
 * @copyright Copyright (c) 2025
 */

#include "err.h"
#include "dev/analog.h"
#include "dev/gpio_signals.h"
#include "sys/yield.h"
#include "sys/sleep.h"

/* converter pin */
#define GPIO_EN                         (gpio_signal_t)GPIO_SIGNAL_B4
/* current measurement channel */
#define ANALOG_CH_ISENSE                ANALOG_IN2

/* is the dc/dc enabled? */
static int enabled;

/* initialize step up converter control */
err_t StepUp_Init(void)
{
    /* prepare the pin */
    GPIOSig_CfgOutput(GPIO_EN, GPIO_OTYPE_OD, 1);
    /* configure analog channel */
    Analog_ConfigureGPIO(ANALOG_CH_ISENSE);
    Analog_ConfigureChannel(ANALOG_CH_ISENSE, ANALOG_SMPL_TIME_28);

    /* return status */
    return EOK;
}

/* enable/disable step up converter */
err_t StepUp_Enable(int enable)
{
    /* drive the control pin */
    GPIOSig_Set(GPIO_EN, enable ? 0 : 1);
    /* small delay is applicable after enabling */
    if (!enabled && enable)
        Sleep(10);
    /* store the state */
    enabled = enable;

    /* not much can fail here */
    return EOK;
}

/* obtain the current current consumption ;-) */
err_t StepUp_GetCurrentConsumption(float *amps)
{
    float current = 0;
    /* adc conversion variables */
    uint16_t adc_val; uint32_t i = 0, adc_accu = 0;

    /* 6v rail is disabled, current must be zero */
    if (!enabled)
        goto end;

    /* measurement timestamp */
    time_t ts = time(0);
    /* do the adc conversion */
    for (; !i || dtime_now(ts) < 20; i++, adc_accu += adc_val, Yield())
        Analog_Convert(ANALOG_CH_ISENSE, &adc_val);

    /* convert the readout to volts */
    float volts = 3.0f * adc_accu / ANALOG_MAX_VAL / i;
    /* thanks to the gain of the current measuring op amp being equal to
     * 50 and the shunt resistor being 20mOhms we get that 1V measured by the
     * adc corresponds to 1A being drawn */
    current = volts * 1.0f;

    /* store the current */
    end: if (amps) *amps = current;
    /* return status */
    return EOK;
}