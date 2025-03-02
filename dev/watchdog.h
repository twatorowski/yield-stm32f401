/**
 * @file watchdog.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-03-02
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef DEV_WATCHDOG_H
#define DEV_WATCHDOG_H

#include "err.h"

/* watchdog interrupt */
void Watchdog_WWDGIsr(void);


/**
 * @brief initialize watchdog
 *
 * @return err_t error code
 */
err_t Watchdog_Init(void);


/**
 * @brief kick the dog!
 *
 */
void Watchdog_Kick(void);


#endif /* DEV_WATCHDOG_H */
