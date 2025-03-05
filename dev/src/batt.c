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
#include "util/elems.h"

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

/* convert to millivolts to capacity */
int BATT_VoltageToCap(int mv)
{
	/* matching lut entries */
	int l1 = 0, l2 = 0, i = 1;
	/* lookup table for conversion of battery voltages to capacity in 5%
	 * steps for discharging */
	const uint16_t lut[] = {
		4170, 4100, 4050, 4000, 3960, 3915,
		3880, 3845, 3814, 3790, 3770, 3750,
		3740, 3730, 3720, 3700, 3680, 3640,
		3600, 3500, 3400
	};

	/* high voltage :) */
	if (mv >= lut[0])
		return 100;

	/* low voltage :( */
	if (mv <= lut[elems(lut) - 1])
		return 0;

	/* look for appropriate lut entry */
	for (i = 1; i < (int)elems(lut); i++) {
		/* found entry! */
		if (mv >= lut[i]) {
			/* store entries */
			l1 = lut[i-1], l2 = lut[i]; break;
		}
	}

	/* return capacity */
	return 100 - i * 5 + (5 * (mv - l2)) / (l1 - l2);
}