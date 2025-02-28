/**
 * @file batt.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-28
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef DEV_BATT_H
#define DEV_BATT_H


/**
 * @brief initialize battery voltage monitor
 *
 * @return err_t error code
 */
err_t Batt_Init(void);


/**
 * @brief get the battery voltage in mV
 *
 * @param batt_mv battery voltage in mV
 *
 * @return err_t error code
 */
err_t Batt_GetVoltage(float *batt_mv);


#endif /* DEV_BATT_H */
