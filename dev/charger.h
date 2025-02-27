/**
 * @file charger.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-27
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef DEV_CHARGER_H
#define DEV_CHARGER_H

/** enumeration of all possible charging currents */
typedef enum charger_current {
    CHARGER_CURRENT_182MA,
    CHARGER_CURRENT_515MA,
    CHARGER_CURRENT_770MA,
    CHARGER_CURRENT_1103MA,
    CHARGER_CURRENT_1394MA,
    CHARGER_CURRENT_1727MA,
    CHARGER_CURRENT_1982MA,
    CHARGER_CURRENT_2316MA
} charger_current_t;

/**
 * @brief initialize the charger control
 *
 * @return err_t
 */
err_t Charger_Init(void);

/**
 * @brief enable/disable charger
 *
 * @param enable 1 - enable, 0 - disable
 * @return err_t error code
 */
err_t Charger_Enable(int enable);

/**
 * @brief set the charging current
 *
 * @param current charging current level
 *
 * @return err_t error code
 */
err_t Charger_SetChargingCurrent(charger_current_t current);

/**
 * @brief is the charger charging?
 *
 * @return int 1 - yes, 0 - no
 */
int Charger_IsCharging(void);


#endif /* DEV_CHARGER_H */
