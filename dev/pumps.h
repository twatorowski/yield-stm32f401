/**
 * @file pumps.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-28
 *
 * @copyright Copyright (c) 2025
 */

#ifndef DEV_PUMPS_H
#define DEV_PUMPS_H

#include "err.h"

/* pumps present within the system */
typedef enum pumps_pump {
    PUMPS_PUMP_FLUID,
    PUMPS_PUMP_AIR
} pumps_pump_t;

/* pump motor direction */
typedef enum pumps_dir {
    PUMPS_DIR_FWD,
    PUMPS_DIR_BACK,
} pumps_dir_t;

/* initialize pump motor driver support */
err_t Pumps_Init(void);


/**
 * @brief set the duty cycle for any given pump
 *
 * @param pump pump in question
 * @param direction direction of the pump motor
 * @param duty_cycle duty cycle (0.0 - 1.0)
 *
 * @return err_t error code
 */
err_t Pumps_SetPumpDutyCycle(pumps_pump_t pump, pumps_dir_t direction,
    float duty_cycle);

/**
 * @brief get the current drawn by the pump motor
 *
 * @param pump pump in question
 * @param current_a current in amperes
 *
 * @return err_t status
 */
err_t Pumps_GetCurrentDraw(pumps_pump_t pump, float *current_a);



#endif /* DEV_PUMPS_H */
