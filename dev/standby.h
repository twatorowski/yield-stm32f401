/**
 * @file standby.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-03-04
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef DEV_STANDBY_H
#define DEV_STANDBY_H

#include "err.h"

/**
 * @brief  initialize support for the standby mode
 *
 * @return err_t
 */
err_t StandBy_Init(void);


/**
 * @brief enter standby mode
 */
void StandBy_Enter(void);

#endif /* DEV_STANDBY_H */
