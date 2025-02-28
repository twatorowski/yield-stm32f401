/**
 * @file valve.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-28
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef DEV_VALVE_H
#define DEV_VALVE_H


/**
 * @brief initialize valve support
 *
 * @return err_t error code
 */
err_t Valve_Init(void);

/**
 * @brief enable/disable the valve
 *
 * @param enable 1 - enable, 0 - disable
 *
 * @return err_t error code
 */
err_t Valve_Enable(int enable);


#endif /* DEV_VALVE_H */
