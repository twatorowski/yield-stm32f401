/**
 * @file seed.c
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-01-03
 * 
 * @copyright Copyright (c) 2025
 */

#include "assert.h"
#include "err.h"

#include "dev/analog.h"
#include "stm32f401/uid.h"
#include "util/jenkins.h"
#include "util/lfsr32.h"

/* seed value */
static uint32_t seed, generated, rand;

/* generate the seed */
err_t Seed_Init(void)
{
    /* we use 18 readings from analog channels */
    uint16_t vals[19];

    /* scan all analog channels */
    for (analog_channel_t ac = ANALOG_IN0; ac < ANALOG_IN18; ac++) {
        /* configure channel */
        Analog_ConfigureChannel(ac, ANALOG_SMPL_TIME_144);
        /* put gpio into analog mode */
        Analog_ConfigureGPIO(ac);
        /* do the conversion */
        Analog_Convert(ac, &vals[ac]);
    }

    /* convert the temperature sensor value */
    Analog_EnableTempSensor(1); Analog_Convert(ANALOG_IN18, &vals[17]);
    Analog_EnableTempSensor(0);

    /* convert the battery readout */
    Analog_EnableVBatBridge(1); Analog_Convert(ANALOG_IN18, &vals[18]);
    Analog_EnableVBatBridge(0);

    /* compute the jenkins hash over the gathered data */
    seed = Jenkins_OAAT(0, (uint8_t *)vals, sizeof(vals));

    /* get the device unique identifier */
    uint32_t uid[3] = { UID->U_ID0, UID->U_ID1, UID->U_ID2 };
    /* compute the jenkins hash over the uid */
    seed = Jenkins_OAAT(seed, (uint8_t *)uid, sizeof(vals));

    /* we are now officially generated */
    generated = 1; rand = seed;

    /* report status */
    return EOK;
}

/* return the seed value */
uint32_t Seed_GetSeed(void)
{
    /* sanity check */
    assert(generated, "seed value is not generated");
    /* return value */
    return seed;
}

/* return next random value */
uint32_t Seed_GetRand(void)
{
    /* sanity check */
    assert(generated, "seed value is not generated");
    /* return the random value */
    return (rand = LFSR32_Next(rand));
}

/* return random integer from the range a to b (both included) */
int Seed_GetRandInt(int a, int b)
{
    /* invalid argument */
    if (a > b)
        return 0;

    /* get the random value */
    return a + Seed_GetRand() % (b - a + 1);
}