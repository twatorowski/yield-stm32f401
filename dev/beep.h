/**
 * @file beep.h
 * @author Tomasz Watorowski (tomasz.watorowski@gmail.com)
 * @date 2025-02-27
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef DEV_BEEP_H
#define DEV_BEEP_H

#include "err.h"


/**
 * @brief initialize beeper
 *
 * @return err_t error code
 */
err_t Beep_Init(void);

/**
 * @brief beep for a certain amount of time
 *
 * @param duration_ms duration expressed in milliseconds
 *
 * @return err_t error code
 */
err_t Beep_Beep(dtime_t duration_ms);

/**
 * @brief set the beeper on or off
 *
 * @param state 1 - on, 0 - off
 *
 * @return err_t error code
 */
err_t Beep_Set(int state);


#endif /* DEV_BEEP_H */
