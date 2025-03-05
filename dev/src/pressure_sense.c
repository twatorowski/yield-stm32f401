/**
 * @file pressure.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-28
 * 
 * @copyright Copyright (c) 2025
 */

#include "err.h"
#include "dev/swi2c.h"
#include "dev/swi2c_dev.h"

#include "dev/nau7802.h"
#include "dev/gpio_signals.h"
#include "sys/sleep.h"
#include "sys/yield.h"

#define DEBUG_LVL DLVL_INFO
#include "debug.h"


/* pressure sensor enable pin */
#define GPIO_PRES_EN                            (gpio_signal_t)GPIO_SIGNAL_B12


/* is the sensor enabled? */
static int enabled;
/* setup device descriptor */
static nau7802_dev_t nau = { .swi2c = &swi2c_nau, .drdy = GPIO_SIGNAL_D2 };

/* initialize the pressure sensor */
err_t PressureSense_Init(void)
{
    /* configure enable pin */
    GPIOSig_CfgOutput(GPIO_PRES_EN, GPIO_OTYPE_OD, 1);

    /* return status */
    return EOK;
}

/* enable/disable the pressure sensor */
err_t PressureSense_Enable(int enable)
{
    /* enable uses negated logic */
    GPIOSig_Set(GPIO_PRES_EN, enable ? 0 : 1);
    /* give the sensor some time to wake up */
    if (!enabled && enable) {
        /* let the voltage stabilize */
        Sleep(100);
        /* reset the i2c */
        SwI2C_Reset(nau.swi2c);
        /* configure the adc */
        NAU7802_DevInit(&nau);
        /* set the sampling rate */
        NAU7802_SetSamplingRate(&nau, NAU7802_RATE_10SPS);
        /* enable the adc */
        NAU7802_Enable(&nau, enable);
    }
    /* store the flag */
    enabled = enable;


    /* report the status */
    return EOK;
}

/* enable/disable the pressure sensor */
err_t PressureSense_GetReadout(float *pressure_kpa)
{
    /* error code */
    err_t ec; int32_t adc_value; int data_ready;

    /* timeout support */
    for (time_t ts = time(0); dtime_now(ts) < 1000; Yield()) {
        /* pressure sensor is not enabled */
        if (!enabled) {
            dprintf_i("sensor not enabled\n", 0);
            return EFATAL;
        }

        /* check the data ready status */
        if ((ec = NAU7802_DataReady(&nau, &data_ready)) < EOK) {
            dprintf_i("unable to read data ready\n", 0);
            return ec;
        }
        /* data is still not ready */
        if (!data_ready)
            continue;

        /* obtain the sample */
        if ((ec = NAU7802_Read(&nau, &adc_value)) < EOK) {
            dprintf_i("unable to read pressure\n", 0);
            return ec;
        }

        /* convert the readout to millivolts */
        float mv = 1500.f * adc_value / NAU7802_MAX_VAL / 128.f;
        /* full scale [100 kpa] is about 100mV */
        float kpa = mv * (100.f / 100.f);
        /* store the value and exit the loop */
        if (pressure_kpa)
            *pressure_kpa = kpa;
        /* break the loop */
        return EOK;
    }

    /* timeout has occured */
    return ETIMEOUT;
}
