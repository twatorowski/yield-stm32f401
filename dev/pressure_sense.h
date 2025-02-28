/**
 * @file pressure_sense.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-28
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef DEV_PRESSURE_SENSE_H
#define DEV_PRESSURE_SENSE_H

#include "err.h"

/**
 * @brief initialize the pressure sensor
 *
 * @return err_t
 */
err_t PressureSense_Init(void);

/**
 * @brief enable/disable the pressure sensor
 *
 * @param enable enable the sensor
 * @return err_t error code
 */
err_t PressureSense_Enable(int enable);


/**
 * @brief enable/disable the pressure sensor
 *
 * @param pressure_kpa pressure readout
 *
 * @return err_t error code
 */
err_t PressureSense_GetReadout(float *pressure_kpa);


#endif /* DEV_PRESSURE_SENSE_H */
