/**
 * @file batt.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-28
 *
 * @copyright Copyright (c) 2025
 */

#include "err.h"
#include "dev/analog.h"
#include "sys/yield.h"
#include "sys/time.h"

/* analog channels */
#define ANALOG_CH_BATT_U            ANALOG_IN9


/* initialize battery voltage monitor */
err_t Batt_Init(void)
{
    /* configure sampling time */
    Analog_ConfigureGPIO(ANALOG_CH_BATT_U);
    Analog_ConfigureChannel(ANALOG_CH_BATT_U, ANALOG_SMPL_TIME_112);

    /* report status */
    return EOK;
}

/* get the battery voltage in mV */
err_t Batt_GetVoltage(float *batt_mv)
{
    /* conversion value */
    uint16_t adc_val; uint32_t i = 0, adc_accu = 0;

    /* current timestamp */
    time_t ts = time(0);
    /* we measure the current by sampling the adc during 2 timer cycles
     * looking for the maximal value */
    for (; dtime_now(ts) < 20 || !i; i++, adc_accu += adc_val, Yield())
        Analog_Convert(ANALOG_CH_BATT_U, &adc_val);

    /* since we use the resistive divider to scale down the battery voltage
     * by a factor of 0.5 we need to multiply the result by 1/0.5 =  2.0 */
    float mv = 2.0f * 3000.f * adc_accu / ANALOG_MAX_VAL / i;
    /* store the result */
    if (batt_mv) *batt_mv = mv;
    /* return status */
    return EOK;
}
