/**
 * @file adc.h
 * 
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2021-04-18
 * 
 * @brief Analog to digital converter
 */

#ifndef _DEV_ANALOG_H
#define _DEV_ANALOG_H

#include <stdint.h>

#include "err.h"
#include "stm32f401/adc.h"

/* maximal value reported by adc */
#define ANALOG_MAX_VAL                      4095

/** channels that are present on board */
typedef enum analog_channel {
    ANALOG_IN0,
    ANALOG_IN1,
    ANALOG_IN2,
    ANALOG_IN3,
    ANALOG_IN4,
    ANALOG_IN5,
    ANALOG_IN6,
    ANALOG_IN7,
    ANALOG_IN8,
    ANALOG_IN9,
    ANALOG_IN10,
    ANALOG_IN11,
    ANALOG_IN12,
    ANALOG_IN13,
    ANALOG_IN14,
    ANALOG_IN15,
    ANALOG_IN16,
    ANALOG_IN17,
    ANALOG_IN18,
} analog_channel_t;

/** analog channel sampling times  */
typedef enum analog_sampling_time {
    ANALOG_SMPL_TIME_3,
    ANALOG_SMPL_TIME_15,
    ANALOG_SMPL_TIME_28,
    ANALOG_SMPL_TIME_56,
    ANALOG_SMPL_TIME_84,
    ANALOG_SMPL_TIME_112,
    ANALOG_SMPL_TIME_144,
    ANALOG_SMPL_TIME_480,
} analog_sampling_time_t;

/**
 * @brief initialize the analog to digital conversion driver
 * 
 * @return err_t init status
 */
err_t Analog_Init(void);

/**
 * @brief configure channel sampling time
 * 
 * @param channel adc channel number
 * @param sampling_time number of clock cycles to sample the channel
 * 
 * @return err_t error code;
 */
err_t Analog_ConfigureChannel(analog_channel_t channel,
    analog_sampling_time_t sampling_time);

/**
 * @brief configure gpio pin associated with given analog channel
 * to work in analog mode
 *
 * @param channel analog channel
 * @return err_t error code
 */
err_t Analog_ConfigureGPIO(analog_channel_t channel);

/**
 * @brief performs convrsion of given analog channel
 * 
 * @param channel channel number
 * @param value placeholder for the converted value
 * 
 * @return err_t conversion status
 */
err_t Analog_Convert(analog_channel_t channel, uint16_t *value);


/**
 * @brief enable internal temperature sensor and connect it to the adc_in18
 *
 * @param enable 1 - enable, 0 - disable
 *
 * @return err_t error code
 */
err_t Analog_EnableTempSensor(int enable);

/**
 * @brief enable internal temperature sensor and connect it to the adc_in18
 *
 * @param enable 1 - enable, 0 - disable
 *
 * @return err_t error code
 */
err_t Analog_EnableVBatBridge(int enable);

#endif /* _DEV_ADC_H */
